project "GloryRuntime"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",

		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
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
		"GloryJobs",
		"GloryThreads",

		"GloryReflect",
		"GloryUtils",
		"GloryECS",
		"GloryUtilsVersion",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_RUNTIME_EXPORTS",
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v143"

		defines
		{
			"_LIB"
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
