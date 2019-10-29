#include "gepch.h"
#include "PerspectiveCamera.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace Engine {

	PerspectiveCamera::PerspectiveCamera(glm::vec3 position, float aspectRatio, glm::vec3 up, float yaw, float pitch)
		: m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(2.5f), m_MouseSensitivity(0.1f), m_AspectRatio(aspectRatio)
	{
		m_Position = position;
		m_WorldUp = up;
		m_Yaw = yaw;
		m_Pitch = pitch;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_ZoomLevel), aspectRatio, 0.1f, 100.0f);
		UpdateCameraVectors();
		RecalculateViewMatrix();
	}
	
	void PerspectiveCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::OnUpdate(Timestep ts)
	{
		float velocity = m_MovementSpeed * ts.GetSeconds();
		if (Input::IsKeyPressed(GE_KEY_W))
			m_Position += m_Front * velocity;
		if (Input::IsKeyPressed(GE_KEY_S))
			m_Position -= m_Front * velocity;
		if (Input::IsKeyPressed(GE_KEY_A))
			m_Position -= m_Right * velocity;
		if (Input::IsKeyPressed(GE_KEY_D))
			m_Position += m_Right * velocity;

		RecalculateViewMatrix();
	}

	void PerspectiveCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>(GE_BIND_EVENT_FN(PerspectiveCamera::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_EVENT_FN(PerspectiveCamera::OnMouseScrolled));
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	bool PerspectiveCamera::OnMouseMoved(MouseMovedEvent& e)
	{
		if (firstMouse)
		{
			lastX = e.GetX();
			lastY = e.GetY();
			firstMouse = false;
		}

		float xoffset = e.GetX() - lastX;
		float yoffset = lastY - e.GetY(); // reversed since y-coordinates range from bottom to top

		lastX = e.GetX();
		lastY = e.GetY();

		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		bool constrainPitch = true;
		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		UpdateCameraVectors();
		RecalculateViewMatrix();
		return true;
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	bool PerspectiveCamera::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 1.5f;
		m_ZoomLevel = std::clamp(m_ZoomLevel, 1.0f, 90.0f);
		SetProjection(m_ZoomLevel);
		return true;
	}

	void PerspectiveCamera::SetProjection(float ZoomLevel)
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(ZoomLevel), m_AspectRatio, 0.1f, 100.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void PerspectiveCamera::UpdateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));
	}

}