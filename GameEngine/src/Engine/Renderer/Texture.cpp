#include "gepch.h"
#include "Texture.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLTexture.h"

namespace Engine {
	
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:		GE_CORE_ASSERT(false, "RendererAPI::None"); return nullptr;
		case RendererAPI::API::OpenGL:		return std::make_shared<OpenGLTexture2D>(path);
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::vector<std::string>& faces)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	return nullptr;
		case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLTextureCube>(faces);
		}
		return nullptr;
	}
}