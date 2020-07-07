#include "gepch.h" 
#include "Mesh.h"

#include <glad/glad.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "imgui.h"

// TMP
#include "stb_image.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Engine {

	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ValidateDataStructure;    // Validation

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			GE_CORE_ERROR("Assimp error: {0}", message);
		}
	};

	static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 &from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	Mesh::Mesh(const std::string& filename)
		: m_FilePath(filename)
	{
		LogStream::Initialize();
		m_Directory = filename.substr(0, filename.find_last_of('/'));

		GE_CORE_INFO("Loading mesh: {0}", filename.c_str());

		m_Importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile(filename, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
			GE_CORE_ERROR("Failed to load mesh file: {0}", filename);

		m_IsAnimated = scene->mAnimations != nullptr;
		m_InverseTransform = glm::inverse(aiMatrix4x4ToGlm(scene->mRootNode->mTransformation));

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		m_Submeshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];

			Submesh submesh;
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;
			//m_Submeshes.push_back(submesh);

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			GE_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
			GE_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

			// Vertices
			if (m_IsAnimated)
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					AnimatedVertex vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_AnimatedVertices.push_back(vertex);
				}
			}
			else
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_StaticVertices.push_back(vertex);
				}
			}

			// Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				GE_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
				m_Indices.push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
			}

			// Materials
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::vector<Tex> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			submesh.Texture.insert(submesh.Texture.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<Tex> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			submesh.Texture.insert(submesh.Texture.end(), specularMaps.begin(), specularMaps.end());

			std::vector<Tex> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			submesh.Texture.insert(submesh.Texture.end(), normalMaps.begin(), normalMaps.end());

			std::vector<Tex> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			submesh.Texture.insert(submesh.Texture.end(), heightMaps.begin(), heightMaps.end());

			m_Submeshes.push_back(submesh);
		}

		GE_CORE_TRACE("NODES:");
		GE_CORE_TRACE("-----------------------------");
		TraverseNodes(scene->mRootNode);
		GE_CORE_TRACE("-----------------------------");

		// Bones
		if (m_IsAnimated)
		{
			for (size_t m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];
				Submesh& submesh = m_Submeshes[m];

				for (size_t i = 0; i < mesh->mNumBones; i++)
				{
					aiBone* bone = mesh->mBones[i];
					std::string boneName(bone->mName.data);
					int boneIndex = 0;

					if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
					{
						// Allocate an index for a new bone
						boneIndex = m_BoneCount;
						m_BoneCount++;
						BoneInfo bi;
						m_BoneInfo.push_back(bi);
						m_BoneInfo[boneIndex].BoneOffset = aiMatrix4x4ToGlm(bone->mOffsetMatrix);
						m_BoneMapping[boneName] = boneIndex;
					}
					else
					{
						GE_CORE_TRACE("Found existing bone in map");
						boneIndex = m_BoneMapping[boneName];
					}

					for (size_t j = 0; j < bone->mNumWeights; j++)
					{
						int VertexID = submesh.BaseVertex + bone->mWeights[j].mVertexId;
						float Weight = bone->mWeights[j].mWeight;
						m_AnimatedVertices[VertexID].AddBoneData(boneIndex, Weight);
					}
				}
			}
		}

		m_VertexArray = VertexArray::Create();

		if (m_IsAnimated)
		{
			auto vb = VertexBuffer::Create(m_AnimatedVertices.data(), m_AnimatedVertices.size() * sizeof(AnimatedVertex));
			vb->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Int4, "a_BoneIDs" },
				{ ShaderDataType::Float4, "a_BoneWeights" },
				});
			m_VertexArray->AddVertexBuffer(vb);
		}
		else
		{
			auto vb = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
			vb->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				});
			m_VertexArray->AddVertexBuffer(vb);
		}
		// OpenGLIndexBuffer was changed to take uint32_t instead of void* and this is the effect
		// also it does m_Count * sizeof(uint32_t) instead of just m_Count to work with sandbox2D
		// OLD: auto ib = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
		auto ib = IndexBuffer::Create(&(m_Indices.data()->V1), m_Indices.size() * 3);
		m_VertexArray->SetIndexBuffer(ib);
		m_Scene = scene;
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::TraverseNodes(aiNode* node, int level)
	{
		std::string levelText;
		for (int i = 0; i < level; i++)
			levelText += "-";
		GE_CORE_TRACE("{0}Node name: {1}", levelText, std::string(node->mName.data));
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			uint32_t mesh = node->mMeshes[i];
			m_Submeshes[mesh].Transform = aiMatrix4x4ToGlm(node->mTransformation);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			aiNode* child = node->mChildren[i];
			TraverseNodes(child, level + 1);
		}
	}

	std::vector<Tex> Mesh::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<Tex> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < m_TexturesLoaded.size(); j++)
			{
				if (std::strcmp(m_TexturesLoaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(m_TexturesLoaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Tex texture;
				texture.id = TextureFromFile(str.C_Str(), m_Directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				m_TexturesLoaded.push_back(texture);
			}
		}
		return textures;
	}

	void Mesh::BoneTransform(float time)
	{
		ReadNodeHierarchy(time, m_Scene->mRootNode, glm::mat4(1.0f));

		m_BoneTransforms.resize(m_BoneCount);

		for (size_t i = 0; i < m_BoneCount; i++)
			m_BoneTransforms[i] = m_BoneInfo[i].FinalTransformation;
	}

	uint32_t Mesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	uint32_t Mesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		GE_CORE_ASSERT(pNodeAnim->mNumRotationKeys > 0, "");

		for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
		{
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	uint32_t Mesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		GE_CORE_ASSERT(pNodeAnim->mNumScalingKeys > 0, "");

		for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		{
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	glm::vec3 Mesh::CalcInterpolatedPosition(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumPositionKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mPositionKeys[0].mValue;
			return { v.x, v.y, v.z };
		}

		uint32_t PositionIndex = FindPosition(animationTime, nodeAnim);
		uint32_t NextPositionIndex = (PositionIndex + 1);
		GE_CORE_ASSERT(NextPositionIndex < nodeAnim->mNumPositionKeys, "");
		float DeltaTime = (float)(nodeAnim->mPositionKeys[NextPositionIndex].mTime - nodeAnim->mPositionKeys[PositionIndex].mTime);
		float Factor = (animationTime - (float)nodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		if (Factor < 0.0f)
			Factor = 0.0f;
		GE_CORE_ASSERT(Factor <= 1.0f, "Factor must be below 1.0f");
		const aiVector3D& Start = nodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = nodeAnim->mPositionKeys[NextPositionIndex].mValue;
		aiVector3D Delta = End - Start;
		auto aiVec = Start + Factor * Delta;
		return { aiVec.x, aiVec.y, aiVec.z };
	}

	glm::quat Mesh::CalcInterpolatedRotation(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumRotationKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mRotationKeys[0].mValue;
			return glm::quat(v.w, v.x, v.y, v.z);
		}

		uint32_t RotationIndex = FindRotation(animationTime, nodeAnim);
		uint32_t NextRotationIndex = (RotationIndex + 1);
		GE_CORE_ASSERT(NextRotationIndex < nodeAnim->mNumRotationKeys, "");
		float DeltaTime = (float)(nodeAnim->mRotationKeys[NextRotationIndex].mTime - nodeAnim->mRotationKeys[RotationIndex].mTime);
		float Factor = (animationTime - (float)nodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		if (Factor < 0.0f)
			Factor = 0.0f;
		GE_CORE_ASSERT(Factor <= 1.0f, "Factor must be below 1.0f");
		const aiQuaternion& StartRotationQ = nodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = nodeAnim->mRotationKeys[NextRotationIndex].mValue;
		auto q = aiQuaternion();
		aiQuaternion::Interpolate(q, StartRotationQ, EndRotationQ, Factor);
		q = q.Normalize();
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::vec3 Mesh::CalcInterpolatedScaling(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mScalingKeys[0].mValue;
			return { v.x, v.y, v.z };
		}

		uint32_t index = FindScaling(animationTime, nodeAnim);
		uint32_t nextIndex = (index + 1);
		GE_CORE_ASSERT(nextIndex < nodeAnim->mNumScalingKeys, "");
		float deltaTime = (float)(nodeAnim->mScalingKeys[nextIndex].mTime - nodeAnim->mScalingKeys[index].mTime);
		float factor = (animationTime - (float)nodeAnim->mScalingKeys[index].mTime) / deltaTime;
		if (factor < 0.0f)
			factor = 0.0f;
		GE_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f");
		const auto& start = nodeAnim->mScalingKeys[index].mValue;
		const auto& end = nodeAnim->mScalingKeys[nextIndex].mValue;
		auto delta = end - start;
		auto aiVec = start + factor * delta;
		return { aiVec.x, aiVec.y, aiVec.z };
	}

	void Mesh::ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
	{
		std::string name(pNode->mName.data);
		const aiAnimation* animation = m_Scene->mAnimations[0];
		glm::mat4 nodeTransform(aiMatrix4x4ToGlm(pNode->mTransformation));
		const aiNodeAnim* nodeAnim = FindNodeAnim(animation, name);

		if (nodeAnim)
		{
			glm::vec3 translation = CalcInterpolatedPosition(AnimationTime, nodeAnim);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

			glm::quat rotation = CalcInterpolatedRotation(AnimationTime, nodeAnim);
			glm::mat4 rotationMatrix = glm::toMat4(rotation);

			glm::vec3 scale = CalcInterpolatedScaling(AnimationTime, nodeAnim);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

			nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
		}

		glm::mat4 transform = ParentTransform * nodeTransform;

		if (m_BoneMapping.find(name) != m_BoneMapping.end())
		{
			uint32_t BoneIndex = m_BoneMapping[name];
			m_BoneInfo[BoneIndex].FinalTransformation = m_InverseTransform * transform * m_BoneInfo[BoneIndex].BoneOffset;
		}

		for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], transform);
	}

	const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
	{
		for (uint32_t i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* pNodeAnim = animation->mChannels[i];
			if (std::string(pNodeAnim->mNodeName.data) == nodeName)
				return pNodeAnim;
		}
		return nullptr;
	}

	void Mesh::Render(Timestep ts, const Ref<Shader>& shader, const glm::mat4& transform)
	{
		if (m_IsAnimated)
		{
			if (m_AnimationPlaying)
			{
				float ticksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f) * m_TimeMultiplier;
				m_AnimationTime += ts * ticksPerSecond;
				m_AnimationTime = fmod(m_AnimationTime, (float)m_Scene->mAnimations[0]->mDuration);
			}

			BoneTransform(m_AnimationTime);
		}

		if (shader)
			shader->Bind();

		// TODO: Sort this out
		m_VertexArray->Bind();

		// TODO: replace with render API calls
		for (Submesh& submesh : m_Submeshes)
		{
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			for (unsigned int i = 0; i < submesh.Texture.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				std::string number;
				std::string name = submesh.Texture[i].type;
				if (name == "texture_diffuse")
					number = std::to_string(diffuseNr++);
				else if (name == "texture_specular")
					number = std::to_string(specularNr++);

				std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->UploadUniformFloat((name + number).c_str(), i);

				glBindTexture(GL_TEXTURE_2D, submesh.Texture[i].id);
			}
			glActiveTexture(GL_TEXTURE0);

			if (m_IsAnimated)
			{
				for (size_t i = 0; i < m_BoneTransforms.size(); i++)
				{
					m_BoneTransforms[i] = m_BoneInfo[i].FinalTransformation;

					std::string uniformName = std::string("u_BoneTransforms[") + std::to_string(i) + std::string("]");
					std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->UploadUniformMat4(uniformName.c_str(), m_BoneTransforms[i]);
				}
			}

			std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->UploadUniformMat4("u_ModelMatrix", transform);
			//std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->UploadUniformMat4("u_ModelMatrix", transform * submesh.Transform);
			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
		}
	}

	void Mesh::OnImGuiRender()
	{
		ImGui::Begin("Mesh Debug");
		if (ImGui::CollapsingHeader(m_FilePath.c_str()))
		{
			if (m_IsAnimated)
			{
				if (ImGui::CollapsingHeader("Animation"))
				{
					if (ImGui::Button(m_AnimationPlaying ? "Pause" : "Play"))
						m_AnimationPlaying = !m_AnimationPlaying;

					ImGui::SliderFloat("##AnimationTime", &m_AnimationTime, 0.0f, (float)m_Scene->mAnimations[0]->mDuration);
					ImGui::DragFloat("Time Scale", &m_TimeMultiplier, 0.05f, 0.0f, 10.0f);
				}
			}
		}

		ImGui::End();
	}

	// TMP
	unsigned int TextureFromFile(const char* path, const std::string& directory)
	{
		std::string filename = path;
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		stbi_set_flip_vertically_on_load(0);
		int width, height, bpp;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &bpp, 0);
		if (data)
		{
			GLenum format;
			if (bpp == 1)
				format = GL_RED;
			else if (bpp == 3)
				format = GL_RGB;
			else if (bpp == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}
		return textureID;
	}
}