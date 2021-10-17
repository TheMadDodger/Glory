project "GlorySDLWindow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Core"] = { "SDLWindow.h", "SDLWindow.cpp", "SDLWindowExceptions.h" }
		["Module"] = { "SDLWindowModule.h", "SDLWindowModule.cpp" }
	}

	includedirs
	{
		"%(vulkan_sdk)\third-party\include",
		"$(SolutionDir)bin\assimp\include"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v142"

		defines
		{
			"_LIB"
		}

	filter "platforms:x86"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

