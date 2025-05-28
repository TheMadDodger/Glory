project "GloryUIMonoExtender"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryUIRenderer/Scripting/csharp")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		["API"] = { "UIComponentsCSAPI.*", "UISceneCSAPI.*" },
		["Extenstion"] = { "UIMonoExtender.*" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.ui}",
		"%{BaseIncludeDir.localize}",
		"%{GloryIncludeDir.mono}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
	}

	links
	{
		"GloryCore",
		"GloryUIRenderer",
		"GloryMonoScripting",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",

		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
		"GloryUtils",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_UUID_DEFINED"
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
