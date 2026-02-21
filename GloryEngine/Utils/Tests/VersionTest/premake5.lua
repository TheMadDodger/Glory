project "VersionTest"
	language "C++"
	cppdialect "C++latest"
	staticruntime "Off"
	kind "ConsoleApp"
	debugdir "%{engineOutDir}"
	--debugargs { "-dataPath=\"%{demosDir}/Sponza/Build/Data\"" }

	targetdir ("%{engineOutDir}/Tests")
	objdir ("%{outputDir}")

	files
	{
		"*.h",
		"*.cpp",
		"premake5.lua"
	}

	includedirs
	{
		"%{IncludeDir.TestFramework}",
		"%{IncludeDir.CommandLine}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.CommandLine}",
	}

	links
	{
		"GloryTestFramework",
		"GloryCommandLine",
		"GloryUtilsVersion"
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
