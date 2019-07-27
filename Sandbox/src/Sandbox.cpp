#include <Engine.h>
#include "imgui/imgui.h"

class ExampleLayer : public GameEngine::Layer
{
public:
	ExampleLayer()
		:Layer("Example") 
	{
		
	}

	void OnUpdate() override
	{
		if (GameEngine::Input::IsKeyPressed(GE_KEY_TAB))
			GE_TRACE("TAB key is pressed!");

	}

	void OnImGuiRender() override
	{
		ImGui::Begin("test sandbox");
		ImGui::Text("text");
		ImGui::End();

	}

	void OnEvent(GameEngine::Event& event) override
	{
		//GE_TRACE("{0}", event);
	}
};

class Sandbox : public GameEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

GameEngine::Application* GameEngine::CreateApplication()
{
	return new Sandbox();
}