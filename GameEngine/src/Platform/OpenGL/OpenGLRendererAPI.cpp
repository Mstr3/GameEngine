#include "gepch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Engine {

	void OpenGLRendererAPI::Init()
	{
		// TODO specific render profiler
		GE_PROFILE_FUNCTION();

		// TODO Depth test toggle func
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Engine::Ref<VertexArray>& vertexArray, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		
		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::DrawArrays(const Engine::Ref<VertexArray>& vertexArray, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}
}