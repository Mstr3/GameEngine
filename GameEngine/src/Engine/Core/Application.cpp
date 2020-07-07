#include "gepch.h"
#include "Application.h"

#include "Engine/Core/Log.h"

#include "Engine/Renderer/Renderer.h"

#include "Input.h"

#include "KeyCodes.h"

#include <GLFW/glfw3.h>

namespace Engine {

// "this" is a hidden parameter for Application::OnEvent()
// e.g. app.OnEvent(e) == OnEvent(this, e)
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		GE_PROFILE_FUNCTION();
		std::string filepath = "testfilepath";
		GE_CORE_ASSERT(!s_Instance, "Application already exists");

		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		GE_PROFILE_FUNCTION();

		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		// If the event you are trying to dispatch e.GetEventType() 
		// matches the type of the function <WindowCloseEvent>
		// then it will run that function
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (m_Running)
		{
			GE_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime(); // Platform::GetTime()
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					GE_PROFILE_SCOPE("LayerStack OnUpdate");

					// LayerStack has begin() and end() implementations
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_Timestep);
				}
			}

			m_ImGuiLayer->Begin();
			{
				GE_PROFILE_SCOPE("LayerStack OnImguiRender");
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();	
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		GE_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case GE_KEY_ESCAPE:
			{
				Close();
				return true;
			}
		}
		return false;
	}
}