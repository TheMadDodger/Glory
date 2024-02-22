project "GloryRuntimeApplication"
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
		"%{engineDir}/GloryEngineBuilder",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.runtime}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",

		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
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
		"GloryCore",
		"GloryEngineBuilder",
		"GloryReflect",
		"GloryRuntime",
		"GloryJobs",
		"GloryThreads",

		"GloryECS",
		"GloryUtils",
		"GloryUtilsVersion",
		"yaml-cpp",
	}

	dependson
	{
		"GloryClusteredRenderer",
		"GloryOpenGLGraphics",
		--"GloryVulkanGraphics",
		"GlorySDLWindow",
		"GlorySDLInput",
		"GloryJoltPhysics",
		--"GloryEditorVulkan",
		"GloryMonoScripting",
		"GloryEngine.Core",
		"GloryEngine.Entities",
		"CSAPIInstall",
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
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
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{engineOutDir}"
		debugargs { "-path=\"%{demosDir}/Sponza/Build/Data/16787318667428770552.gcag\"" }

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
