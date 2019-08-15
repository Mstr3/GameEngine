#pragma once

#include <memory>
#include "Engine/Renderer/Buffer.h"

namespace Engine {

	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const Engine::Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Engine::Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<Engine::Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Engine::Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static VertexArray* Create();
	};
}