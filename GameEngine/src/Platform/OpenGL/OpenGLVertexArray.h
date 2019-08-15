#pragma once

#include "Engine/Renderer/VertexArray.h"

namespace Engine {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Engine::Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Engine::Ref<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Engine::Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; };
		virtual const Engine::Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; };
	private:
		uint32_t m_RendererID;
		std::vector<Engine::Ref<VertexBuffer>> m_VertexBuffers;
		Engine::Ref<IndexBuffer> m_IndexBuffer;
	};
}