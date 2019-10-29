#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Engine/Core/Timestep.h"
#include "Engine/Core/Core.h"

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Shader.h"

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;
struct aiMaterial;
enum aiTextureType;

namespace Assimp {
	class Importer;
}

namespace Engine {

unsigned int TextureFromFile(const char* path, const std::string& directory);

#define NUM_BONES_PER_VEREX 4

	struct VertexBoneData
	{
		uint32_t IDs[NUM_BONES_PER_VEREX];
		float Weights[NUM_BONES_PER_VEREX];

		VertexBoneData()
		{
			memset(IDs, 0, sizeof(IDs));
			memset(Weights, 0, sizeof(Weights));
		};

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (uint32_t i = 0; i < NUM_BONES_PER_VEREX; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// should never get here - more bones than we have space for
			GE_CORE_ASSERT(false, "Too many bones (max 4)");
		}
	};

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 Texcoord;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		// 4 uint32_t ids / 4 float weights
		uint32_t IDs[4] = { 0, 0,0, 0 };
		float Weights[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// TODO: Keep top weights
			GE_CORE_WARN("Vertex has more than four bones/weights affecting it, extra data will be discarded (BoneID={0}, Weight={1})", BoneID, Weight);
		}
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 Texcoord;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
	};

	struct Index
	{
		uint32_t V1, V2, V3;
	};

	static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

	// move it to texture type
	struct Tex
	{
		unsigned int id;
		std::string type;
		std::string path;
	};

	class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;
		std::vector<Tex> Texture;

		glm::mat4 Transform;
	};

	class Mesh
	{
	public:
		Mesh(const std::string& filename);
		~Mesh();

		void Render(Timestep ts, const Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));
		void OnImGuiRender();

		inline Ref<Shader> GetMeshShader() { return m_MeshShader; }
		inline const std::string& GetFilePath() const { return m_FilePath; }
	private:
		std::vector<Tex> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

		void BoneTransform(float time);
		void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);

		const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		void TraverseNodes(aiNode* node, int level = 0);

		uint32_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 CalcInterpolatedPosition(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat CalcInterpolatedRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 CalcInterpolatedScaling(float animationTime, const aiNodeAnim* nodeAnim);
	private:
		std::vector<Tex> m_TexturesLoaded;
		std::vector<Submesh> m_Submeshes;
		std::string m_Directory;

		std::unique_ptr<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;
		std::vector<BoneInfo> m_BoneInfo;
		std::vector<glm::mat4> m_BoneTransforms;
		std::unordered_map<std::string, uint32_t> m_BoneMapping;
		uint32_t m_BoneCount = 0;
		
		bool m_IsAnimated;
		float m_AnimationTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool m_AnimationPlaying = true;

		Ref<VertexArray> m_VertexArray;

		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Vertex> m_StaticVertices;
		std::vector<Index> m_Indices;
		const aiScene* m_Scene;

		// Materials
		Ref<Shader> m_MeshShader;

		std::string m_FilePath;
	};
}