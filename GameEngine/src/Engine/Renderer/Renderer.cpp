#include "gepch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Engine {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		GE_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::BeginScene(PerspectiveCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();
	}

	void Renderer::BeginScene(Camera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Engine::Ref<Shader>& shader, const Engine::Ref<VertexArray>& vertexArray, const glm::mat4& transform, bool depthTest)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjectionMatrix", s_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ModelMatrix", transform);

		vertexArray->Bind();
		//TODO: DepthTest needs to be called only with depthTest = false
		if (vertexArray->GetIndexBuffer())
		{
			RenderCommand::DepthTest(depthTest);
			RenderCommand::DrawIndexed(vertexArray);
			RenderCommand::DepthTest(true);
		}
		else
		{
			RenderCommand::DepthTest(depthTest);
			RenderCommand::DrawArrays(vertexArray);
			RenderCommand::DepthTest(true);
		}
	}
}