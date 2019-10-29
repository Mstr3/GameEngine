#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	Engine::Application* app = Engine::CreateApplication();
	GE_CORE_ASSERT(app, "Client Application is null!");
	app->Run();
	delete app;
}

#endif