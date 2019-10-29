#include "gepch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Engine {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		RenderCommand::Init();
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
		if(vertexArray->GetIndexBuffer())
			RenderCommand::DrawIndexed(vertexArray, depthTest);
		else
			RenderCommand::DrawArrays(vertexArray, depthTest);
	}
}