workspace "GameEngine"
	architecture "x64"
	startproject "Sandbox3D"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "GameEngine/vendor/GLFW/include"
IncludeDir["GLAD"] = "GameEngine/vendor/GLAD/include"
IncludeDir["ImGui"] = "GameEngine/vendor/imgui"
IncludeDir["glm"] = "GameEngine/vendor/glm"
IncludeDir["stb_image"] = "GameEngine/vendor/stb_image"

-- Include premake5.lua for GLFW (same as c++ #include)
include "GameEngine/vendor/GLFW"
include "GameEngine/vendor/GLAD"
include "GameEngine/vendor/imgui"

project "GameEngine"
	location "GameEngine"
	kind "StaticLib" -- exe, dll etc shared = dynamic
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "gepch.h"
	pchsource "GameEngine/src/gepch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"

	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/spdlog",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{prj.name}/vendor/assimp/include"

	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"opengl32.lib"

	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GE_BUILD_DLL",
			"GE_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			-- ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox2D/\"")
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox2D"
	location "Sandbox2D"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"GameEngine/vendor/spdlog/include",
		"GameEngine/src",
		"GameEngine/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GameEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox3D"
	location "Sandbox3D"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"GameEngine/vendor/spdlog/include",
		"GameEngine/src",
		"GameEngine/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GameEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "on"

		links
        {
            "GameEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
        }

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {
            "GameEngine/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }


	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "on"

		links
        {
            "GameEngine/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }

