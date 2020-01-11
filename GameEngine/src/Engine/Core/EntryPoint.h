#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();

	GE_PROFILE_BEGIN_SESSION("Startup", "EngineProfile-Startup.json");
	Engine::Application* app = Engine::CreateApplication();
	GE_PROFILE_END_SESSION();
	GE_CORE_ASSERT(app, "Client Application is null!");

	GE_PROFILE_BEGIN_SESSION("Runtime", "EngineProfile-Runtime.json");
	app->Run();
	GE_PROFILE_END_SESSION();

	GE_PROFILE_BEGIN_SESSION("Shutdown", "EngineProfile-Shutdown.json");
	delete app;
	GE_PROFILE_END_SESSION();
}

#endif