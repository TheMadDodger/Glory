project "GloriousTestApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files
	{
		"*.h",
		"*.cpp",
		"EditorAssets/**.*",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.editor}",

		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.CommandLine}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryReflect",
		"GloryUtils",
		"GloryUtilsVersion",
		"GloryCommandLine",

		"ImGui",
		"yaml-cpp",
	}

	dependson
	{
		"Glorious",
	}

	postbuildcommands
	{
		("{COPY} %{EditorTestDir}/TestProjects %{engineOutDir}/Tests/Editor/TestProjects"),
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkanDir}/Lib32",
			"%{vulkanDir}/Third-Party/Bin32"
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Lib",
			"%{vulkanDir}/Third-Party/Bin"
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{engineOutDir}"
		debugargs { "-projectPath=\"%{EditorTestDir}/TestProjects/Basic/Basic.gproj\"" }

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
