project "GloryRuntimeApplication"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files
	{
		"*.h",
		"*.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.runtime}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",

		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.CommandLine}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.glm}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryCore",
		"GloryReflect",
		"GloryRuntime",
		"GloryJobs",
		"GloryThreads",

		"GloryECS",
		"GloryUtils",
		"GloryUtilsVersion",
		"GloryCommandLine",
		"yaml-cpp",
	}

	dependson
	{
		"GloryFSM",
		"GloryJoltPhysics",
		"GloryLocalize",
		"GloryMonoScripting",
		"GloryOpenGLGraphics",
		"GloryOverlayConsole",
		"GloryRenderer",
		"GlorySDLAudio",
		"GlorySDLInput",
		"GlorySDLWindow",
		"GlorySteamAudio",
		"GloryUIRenderer",
		"GloryVulkanGraphics",

		"GloryEngine.Core",
		"GloryEngine.FSM",
		"GloryEngine.Jolt",
		"GloryEngine.Localize",
		"GloryEngine.UI",
		"CSAPIInstall",
	}

	defines
	{
		"GLORY_RUNTIME_APPLICATION_EXPORTS"
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
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{engineOutDir}"
		debugargs { "-path=\"%{demosDir}/Sponza/Build/Data/16787318667428770552.gcag\"" }

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
