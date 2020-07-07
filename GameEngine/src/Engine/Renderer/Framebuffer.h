#pragma once

#include "Engine/Core/Base.h"

namespace Engine {

	enum class FramebufferFormat
	{
		None = 0,
		RGBA8 = 1,
		RGBA16F = 2
	};

	class Framebuffer
	{
	public:
		static Ref<Framebuffer> Create(uint32_t width, uint32_t height, FramebufferFormat format);

		virtual ~Framebuffer() {}
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void BindTexture(uint32_t slot = 0) const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetColorAttachmentRendererID() const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
	};
}