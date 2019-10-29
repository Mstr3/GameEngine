#include "gepch.h"
#include "Framebuffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Engine {

	Ref<Engine::Framebuffer> Framebuffer::Create(uint32_t width, uint32_t height, FramebufferFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:		return nullptr;
		case RendererAPI::API::OpenGL:		return std::make_shared<OpenGLFramebuffer>(width, height, format);
		}
		return nullptr;
	}
}