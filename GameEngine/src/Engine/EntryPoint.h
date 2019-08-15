#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	GE_CORE_WARN("Warn log");
	int a = 5;
	GE_INFO("Info log Var={0}", a);


	auto app = Engine::CreateApplication();
	app->Run();
	delete app;
}

#endif