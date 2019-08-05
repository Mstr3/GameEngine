#include "gepch.h"
#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

#include "Renderer.h"

namespace GameEngine {

	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:		GE_CORE_ASSERT(false, "RendererAPI::None"); return nullptr;
		case RendererAPI::API::OpenGL:		return new OpenGLVertexArray();
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
	
}
