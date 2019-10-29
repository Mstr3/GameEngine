#pragma once

#include "RenderCommand.h"

#include "OrthographicCamera.h"
#include "PerspectiveCamera.h"
#include "Camera.h"
#include "Shader.h"

namespace Engine {

	class Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(OrthographicCamera& camera);
		static void BeginScene(PerspectiveCamera& camera);
		static void BeginScene(Camera& camera);
		static void EndScene();

		static void Submit(const Engine::Ref<Shader>& shader, const Engine::Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f), bool depthTest = true);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); };
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> s_SceneData;
	};
}