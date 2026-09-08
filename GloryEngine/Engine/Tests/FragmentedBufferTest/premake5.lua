project "FragmentedBufferTest"
	language "C++"
	cppdialect "C++23"
	staticruntime "Off"
	kind "ConsoleApp"
	debugdir "%{engineOutDir}"

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
		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",

		"%{IncludeDir.TestFramework}",
		"%{IncludeDir.CommandLine}",
		"%{IncludeDir.Version}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",

		"GloryTestFramework",
		"GloryCommandLine",
		"GloryUtilsVersion"
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
