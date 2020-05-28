#pragma once

#include "Engine/Renderer/Texture.h"
#include <glad/glad.h>

namespace Engine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};

	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(const std::vector<std::string>& faces);
		virtual ~OpenGLTextureCube();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		uint32_t m_RendererID;
		uint32_t m_Width;
		uint32_t m_Height;
	};
}