#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "Sandbox2D.h"

class ExampleLayer : public Engine::Layer
{
public:
	ExampleLayer()
		:Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		m_VertexArray = Engine::VertexArray::Create();

		float vertices[7 * 3] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		Engine::Ref<Engine::VertexBuffer> vertexBuffer;
		vertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));


		Engine::BufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position" },
			{Engine::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		Engine::Ref<Engine::IndexBuffer> indexBuffer;
		indexBuffer = Engine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA = Engine::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Engine::Ref<Engine::VertexBuffer> squareVB;
		squareVB = Engine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ Engine::ShaderDataType::Float3, "a_Position" },
			{ Engine::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Engine::Ref<Engine::IndexBuffer> squareIB;
		squareIB = Engine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			
			uniform mat4 u_ViewProjectionMatrix;
			uniform mat4 u_ModelMatrix;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			
			out vec4 color;

			in vec3 v_Position;			
			in vec4 v_Color;			

			void main()
			{
				color = v_Color;
				//color = vec4(v_Position * 0.5 + 0.5, 1.0);
			}
		)";

		// same as make_unique
		m_Shader = Engine::Shader::Create("VertexPosColor",vertexSrc, fragmentSrc);


		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			
			uniform mat4 u_ViewProjectionMatrix;
			uniform mat4 u_ModelMatrix;

			out vec3 v_Position;
			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			
			out vec4 color;

			in vec3 v_Position;		
			
			uniform vec3 u_Color;	

			void main()
			{
				color = vec4(u_Color, 1.0f);
			}
		)";

		// same as make_unique
		m_FlatColorShader = Engine::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/Shaders/Texture.glsl");

		m_Texture = Engine::Texture2D::Create("assets/Textures/Checkerboard.png");

		// tmp until material system 8:43 Shader abstraction
		std::dynamic_pointer_cast<Engine::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Engine::Timestep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Engine::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Engine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_EarthTexture->Bind();
		Engine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		Engine::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Engine::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

private:
	Engine::ShaderLibrary m_ShaderLibrary;
	Engine::Ref<Engine::Shader> m_Shader;
	Engine::Ref<Engine::VertexArray> m_VertexArray;

	Engine::Ref<Engine::Shader> m_FlatColorShader;
	Engine::Ref<Engine::VertexArray> m_SquareVA;

	Engine::Ref<Engine::Texture2D> m_Texture, m_EarthTexture;

	Engine::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Engine::Application
{
public:
	Sandbox(const Engine::ApplicationProps& props)
		: Application(props)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox({"Sandbox app", 1280, 720});
}