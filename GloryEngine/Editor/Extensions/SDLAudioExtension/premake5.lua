project "SDLAudioExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GlorySDLAudio/Editor/Extension")
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

		"%{BaseIncludeDir.audio}",
		"%{BaseIncludeDir.audio3d}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.serialization}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.sdlaudio}",
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

		"%{modulesOutDir}/GlorySDLAudio",
	}

	links
	{
		"SDL2",
		"SDL2_mixer",

		"GloryEngineCore",
		"GloryEngine",
		"GlorySDLAudio",
		"GloryEditor",
		"GloryEditorSerialization",
		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",

		"ImGui",
		"ImGuiTestEngine",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_SDL_AUDIO_EXTENSION_EXPORTS"
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
