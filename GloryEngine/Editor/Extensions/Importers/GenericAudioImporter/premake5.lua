project "GenericAudioImporter"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{engineOutDir}/Extensions")
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
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.editor}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.FA}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.implot}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryECS",
		"GloryReflect",
		"GloryUtils",
		"GloryUtilsVersion",

		"ImGui",
		"ImGuiTestEngine",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_AUDIO_EXTENSION_EXPORTS"
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
