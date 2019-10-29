#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Core/Timestep.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace Engine {

	class PerspectiveCamera
	{
	public:
		PerspectiveCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float aspectRatio = 16 / 9, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		
		void SetProjection(float ZoomLevel);
		void SetPosition(const glm::vec3& position) { m_Position = position; void RecalculateViewMatrix(); }

		glm::vec3& GetPosition() { return m_Position; }
		glm::mat4& GetProjectionMatrix() { return m_ProjectionMatrix; }
		glm::mat4& GetViewMatrix() { return m_ViewMatrix; }
		glm::mat4& GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
		void UpdateCameraVectors();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Front;
		glm::vec3 m_Up;
		glm::vec3 m_Right;
		glm::vec3 m_WorldUp;

		// Euler Angles
		float m_Yaw;
		float m_Pitch;
		// Camera options
		float m_MovementSpeed;
		float m_MouseSensitivity;
		float m_ZoomLevel = 45.0f;
		float m_AspectRatio = 16 / 9;
		
		float lastX = 1280.0f / 2.0f;
		float lastY = 720.0f / 2.0f;
		bool firstMouse = true;
	};
}
