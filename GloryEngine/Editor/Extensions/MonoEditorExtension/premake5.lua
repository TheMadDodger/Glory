project "MonoEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryMonoScripting/Editor/Extension")
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

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.serialization}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.mono}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.FA}",

		"%{rapidjson}",

		"%{rootDir}/third-party/tinyfiledialogs"
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.implot}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",

		"%{modulesDir}/GloryMonoScripting",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryEditorSerialization",
		"GloryMonoScripting",
		"GloryECS",
		"GloryReflect",
		"GloryUtils",
		"GloryUtilsVersion",

		"ImGui",
		"implot",
		"ImGuiTestEngine",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",
		"tinyfiledialogs",
	}

	defines
	{
		"GLORY_MONO_EXTENSION_EXPORTS",
		"GLORY_EDITOR_PROP_DRAWERS_EXPORTS"
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

		links
		{
			"efsw-static-debug"
		}

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links
		{
			"efsw-static-release"
		}
