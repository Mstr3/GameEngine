#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TMP
#include "Platform/OpenGL/OpenGLShader.h"
#include "Engine/Renderer/Camera.h"

class TestLayer : public Engine::Layer
{
public:
	TestLayer()
		: Layer("test"), m_Camera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
		//m_Camera(CameraStartingPos, 1280.0f / 720.0f)
	{
	}

	~TestLayer()
	{
	}

	virtual void OnAttach() 
	{
		// Plane
		float vertices[] = {
		-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		 0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		 0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
										
		-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		 0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};
		m_PlaneVAO = Engine::VertexArray::Create();
		Engine::Ref<Engine::VertexBuffer> planeVBO = Engine::VertexBuffer::Create(vertices, sizeof(vertices));
		Engine::BufferLayout layout = {
			{ Engine::ShaderDataType::Float3, "a_Position"},
			{ Engine::ShaderDataType::Float3, "a_Normal"},
			{ Engine::ShaderDataType::Float2, "a_TexCoords"}
		};
		planeVBO->SetLayout(layout);
		m_PlaneVAO->AddVertexBuffer(planeVBO);

		// Shader
		m_SimpleShader = Engine::Shader::Create("res/shaders/ModelStatic.glsl");
		m_ModelShader = Engine::Shader::Create("res/shaders/ModelAnim.glsl");
		m_SkyboxShader = Engine::Shader::Create("res/shaders/Skybox.glsl");
		m_QuadShader = Engine::Shader::Create("res/shaders/QuadPostprocess.glsl");

		// Texture
		m_TextureTest = Engine::Texture2D::Create("res/textures/Checkerboard.png");

		// Model
		m_ModelCharacter.reset(new Engine::Mesh("res/models/model/model.dae"));
		m_ModelM1911.reset(new Engine::Mesh("res/models/m1911/m1911.fbx"));
		m_ModelSphere.reset(new Engine::Mesh("res/models/Sphere1m.fbx"));

		// CubeMap
		std::vector<std::string> faces
		{
			"res/textures/skybox/right.jpg",
			"res/textures/skybox/left.jpg",
			"res/textures/skybox/top.jpg",
			"res/textures/skybox/bottom.jpg",
			"res/textures/skybox/front.jpg",
			"res/textures/skybox/back.jpg"
		};
		m_CubeMap = Engine::TextureCube::Create(faces);

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
		m_Skybox = Engine::VertexArray::Create();
		Engine::Ref<Engine::VertexBuffer> skyboxVBO = Engine::VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
		Engine::BufferLayout skyboxLayout = {
			{ Engine::ShaderDataType::Float3, "a_Position"} 
		};
		skyboxVBO->SetLayout(skyboxLayout);
		m_Skybox->AddVertexBuffer(skyboxVBO);

		m_Framebuffer = Engine::Framebuffer::Create(1280, 720, Engine::FramebufferFormat::RGBA8);
		
		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		float quadVertices[] = { 
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
		};
		m_FinalQuad = Engine::VertexArray::Create();
		Engine::Ref<Engine::VertexBuffer> quadVB = Engine::VertexBuffer::Create(quadVertices, sizeof(quadVertices));
		Engine::BufferLayout quadLayout = {
			{Engine::ShaderDataType::Float2, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoords"}
		};
		quadVB->SetLayout(quadLayout);
		m_FinalQuad->AddVertexBuffer(quadVB);
	}

	virtual void OnUpdate(Engine::Timestep ts) 
	{
		//m_Camera.OnUpdate(ts);
		m_Camera.Update(ts);
		auto projection = m_Camera.GetProjectionMatrix();
		auto view = m_Camera.GetViewMatrix();
		auto viewProjection = projection * view;

		m_Framebuffer->Bind();
		Engine::RenderCommand::SetClearColor(glm::vec4(0.1, 0.2, 0.3, 0.4));
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_Camera);

		// TODO skybox should be rendered last with GL_LEQUAL not glDisable(GL_DEPTH_TEST)
		m_SkyboxShader->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SkyboxShader)->UploadUniformMat4("u_View", glm::mat4(glm::mat3(view)));
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SkyboxShader)->UploadUniformMat4("u_Projection", projection);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SkyboxShader)->UploadUniformInt("skybox", 0);
		m_CubeMap->Bind();
		// TODO glDepthFunc(GL_LEQUAL) before drawing skybox
		Engine::Renderer::Submit(m_SkyboxShader, m_Skybox, glm::mat4(1.0f), false);

		m_SimpleShader->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformMat4("u_ViewProjectionMatrix", viewProjection);

		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat3("viewPos", m_Camera.GetPosition());
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat3("light.position", m_Light.Pos);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat3("light.ambient", m_Light.Ambient);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat3("light.diffuse", m_Light.Diffuse);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat3("light.specular", m_Light.Specular);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat("light.constant", m_Light.Constant);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat("light.linear", m_Light.Linear);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat("light.quadratic", m_Light.Quadratic);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_SimpleShader)->UploadUniformFloat("shininess", m_Light.Shininess);

		// Plane and light texture
		m_TextureTest->Bind();
		Engine::Renderer::Submit(m_SimpleShader, m_PlaneVAO, glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 100.0f)));

		// Light sphere
		m_ModelSphere->Render(ts, m_SimpleShader, glm::translate(glm::mat4(1.0f), m_Light.Pos));

		m_ModelShader->Bind();	
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformMat4("u_ViewProjectionMatrix", viewProjection);
	
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat3("viewPos", m_Camera.GetPosition());
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat3("light.position", m_Light.Pos);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat3("light.ambient", m_Light.Ambient);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat3("light.diffuse", m_Light.Diffuse);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat3("light.specular", m_Light.Specular);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat("light.constant", m_Light.Constant);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat("light.linear", m_Light.Linear);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat("light.quadratic", m_Light.Quadratic);
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_ModelShader)->UploadUniformFloat("shininess", m_Light.Shininess);

		m_ModelCharacter->Render(ts, m_ModelShader, glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0)));

		m_ModelM1911->Render(ts, m_ModelShader, glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(2,2,2)), glm::vec3(-3.5f, 3.0f, 3.5f)));

		// Binds default framebuffer slot 0
		m_Framebuffer->Unbind();

		// Postprocess
		Engine::RenderCommand::SetClearColor(glm::vec4(0.1, 0.2, 0.3, 0.4));
		Engine::RenderCommand::Clear();

		m_Framebuffer->BindTexture();
		m_QuadShader->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_QuadShader)->UploadUniformInt("u_Blur", m_Blur);
		// default value for sampler2D is always 0
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_QuadShader)->UploadUniformInt("u_Texture", 0);
		Engine::Renderer::Submit(m_QuadShader, m_FinalQuad, glm::mat4(1.0f), false);
		Engine::Renderer::EndScene();
	}

	virtual void OnImGuiRender()
	{
		if (m_ModelCharacter)
			m_ModelCharacter->OnImGuiRender();

		if (m_ModelM1911)
			m_ModelM1911->OnImGuiRender();

		ImGui::Begin("Scene Debug");
		if (ImGui::CollapsingHeader("Light"))
		{
			ImGui::SliderFloat3("Position", &m_Light.Pos.x, -50.0f, 50.0f);
			ImGui::SliderFloat3("Ambient Color", &m_Light.Ambient.x, 0.0f, 1.0f);
			ImGui::SliderFloat3("Diffuse Color", &m_Light.Diffuse.x, 0.0f, 1.0f);
			ImGui::SliderFloat3("Specular Color", &m_Light.Specular.x, 0.0f, 1.0f);

			ImGui::SliderFloat("Constant", &m_Light.Constant, 0.0f, 1.0f);
			ImGui::SliderFloat("Linear", &m_Light.Linear, 0.0f, 1.0f);
			ImGui::SliderFloat("Quadratic", &m_Light.Quadratic, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &m_Light.Shininess, 0.0f, 64.0f);
		}
		if (ImGui::CollapsingHeader("Postprocess"))
		{
			if (ImGui::Button(m_Blur ? "Blur: Disable" : "Blur: Enable"))
				m_Blur = !m_Blur;
		}
		ImGui::End();

	}

	virtual void OnEvent(Engine::Event& event) 
	{
		//m_Camera.OnEvent(event);
	}

private:
	glm::vec3 CameraStartingPos = glm::vec3(0.0f, 0.0f, 3.0f);
	//Engine::PerspectiveCamera m_Camera;
	Engine::Camera m_Camera;
	float m_MeshScale = 1.0f;

	Engine::Ref<Engine::VertexArray> m_PlaneVAO , m_Skybox, m_FinalQuad;

	Engine::Ref<Engine::Texture2D> m_TextureTest;
	Engine::Ref<Engine::Texture2D> m_MeshDiffuse;

	Engine::Ref<Engine::Shader> m_ModelShader, m_SkyboxShader, m_QuadShader, m_SimpleShader;
	Engine::Ref<Engine::Mesh> m_ModelCharacter, m_ModelM1911, m_ModelSphere;
	
	struct Light
	{
		glm::vec3 Pos =	{ -3.0f, 6.0f, 2.0f };
		glm::vec3 Ambient = { 0.2f, 0.2f, 0.2f };
		glm::vec3 Diffuse = { 0.5f, 0.5f, 0.5f };
		glm::vec3 Specular = { 1.0f, 1.0f, 1.0f };

		float Constant = 1.0f;
		float Linear = 0.09f;
		float Quadratic = 0.0f; //0.032f;
		float Shininess = 32.0f;
	};
	Light m_Light;

	float m_Blur = false;
	Engine::Ref<Engine::TextureCube> m_CubeMap;

	Engine::Ref<Engine::Framebuffer> m_Framebuffer;
};

class Test : public Engine::Application
{
public:
	Test(const Engine::ApplicationProps& props)
		: Application(props)
	{
		PushLayer(new TestLayer());
	}

	~Test()
	{

	}
};


Engine::Application* Engine::CreateApplication()
{
	return new Test({ "Test", 1280, 720 });
}