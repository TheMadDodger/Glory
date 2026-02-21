project "RunTests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"
	debugdir "%{engineOutDir}"
	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"main.cpp",
		"premake5.lua"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
		symbols "Off"