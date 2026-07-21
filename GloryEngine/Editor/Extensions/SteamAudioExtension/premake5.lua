project "SteamAudioExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GlorySteamAudio/Editor/Extension")
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
		"%{BaseIncludeDir.physics}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.serialization}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.steamaudio}",
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
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",

		"%{modulesOutDir}/GlorySteamAudio",
		"%{moduleBaseOutDir}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"Glory3DAudioModule",
		"GlorySteamAudio",
		"GloryPhysicsModule",
		"GloryEditor",
		"GloryEditorSerialization",
		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",

		"ImGui",
		"implot",
		"ImGuiTestEngine",
		"yaml-cpp",
		"phonon",
	}

	defines
	{
		"GLORY_STEAM_AUDIO_EXTENSION_EXPORTS"
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
