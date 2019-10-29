#include "gepch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Engine {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Engine::ShaderDataType::Float:		return GL_FLOAT;
			case Engine::ShaderDataType::Float2:	return GL_FLOAT;
			case Engine::ShaderDataType::Float3:	return GL_FLOAT;
			case Engine::ShaderDataType::Float4:	return GL_FLOAT;
			case Engine::ShaderDataType::Mat3:		return GL_FLOAT;
			case Engine::ShaderDataType::Mat4:		return GL_FLOAT;
			case Engine::ShaderDataType::Int:		return GL_INT;
			case Engine::ShaderDataType::Int2:		return GL_INT;
			case Engine::ShaderDataType::Int3:		return GL_INT;
			case Engine::ShaderDataType::Int4:		return GL_INT;
			case Engine::ShaderDataType::Bool:		return GL_BOOL;
		}
		
		GE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Engine::Ref<VertexBuffer>& vertexBuffer)
	{
		GE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
			glEnableVertexAttribArray(index);
			if (glBaseType == GL_INT)
			{
				glVertexAttribIPointer(index, element.GetComponentCount(),
					glBaseType,
					layout.GetStride(),
					(const void*)(intptr_t)element.Offset);
			}
			else
			{
				glVertexAttribPointer(index, element.GetComponentCount(),
					glBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(intptr_t)element.Offset);
			}		
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Engine::Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}