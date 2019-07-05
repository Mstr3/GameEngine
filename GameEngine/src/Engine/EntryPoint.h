#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern GameEngine::Application* GameEngine::CreateApplication();

int main(int argc, char** argv)
{
	GameEngine::Log::Init();
	GE_CORE_WARN("Warn log");
	int a = 5;
	GE_INFO("Info log Var={0}", a);


	auto app = GameEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif