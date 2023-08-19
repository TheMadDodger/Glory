project "MonoEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
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

		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.mono}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.mono}",

		"%{monoDir}/include/mono-2.0",

		"%{IncludeDir.FA}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",

		"%{rapidjson}",

		"%{rootDir}/third-party/tinyfiledialogs"
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.implot}",
		"%{LibDirs.glory}",
		"%{modulesDir}/GloryMonoScripting",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"GloryEditor",
		"GloryMonoScripting",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",
		"tinyfiledialogs",
		"GloryECS",
		"GloryReflect",
		"GloryUtils",
		"GloryUtilsVersion",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_EDITOR_EXPORTS",
		"GLORY_EDITOR_EXTENSION_EXPORTS",
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

		libdirs
		{
			"%{monox86Dir}/lib",
		}

		includedirs
		{
			"%{monox86Dir}/include/mono-2.0",
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{monoDir}/lib",
		}

		includedirs
		{
			"%{monoDir}/include/mono-2.0",
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
