project "GloriousEditorTest"
	language "C++"
	cppdialect "C++23"
	staticruntime "Off"
	kind "ConsoleApp"
	debugdir "%{engineOutDir}/Tests/Editor"

	targetdir ("%{engineOutDir}/Tests/Editor")
	objdir ("%{outputDir}")

	files
	{
		"*.h",
		"*.cpp",
		"premake5.lua"
	}

	includedirs
	{
		"%{GloryIncludeDir.enginecore}",

		"%{IncludeDir.TestFramework}",
		"%{IncludeDir.CommandLine}",
		"%{IncludeDir.Utils}",
	}

	links
	{
		"GloryEngineCore",
		"GloryTestFramework",
		"GloryUtils",
		"GloryCommandLine",
	}

	dependson
	{
		"GloriousTestApp",
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

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
