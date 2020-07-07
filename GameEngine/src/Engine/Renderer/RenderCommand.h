#pragma once

#include "RendererAPI.h"

namespace Engine {

	// 13:40 Blending
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Engine::Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
		inline static void DrawArrays(const Engine::Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawArrays(vertexArray);
		}

		inline static void DepthTest(bool depthTest)
		{
			s_RendererAPI->DepthTest(depthTest);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}