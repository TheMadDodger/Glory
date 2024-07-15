project "SteamAudioExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
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

		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",
		"%{GloryIncludeDir.steamaudio}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.ImGui}",

		"%{IncludeDir.FA}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.implot}",
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",

		"%{modulesOutDir}/GlorySteamAudio",
		"%{moduleBaseOutDir}",
	}

	links
	{
		"GloryCore",
		"GlorySteamAudio",
		"GloryPhysicsModule",
		"GloryEditor",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",

		"phonon",

		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
		"GloryUtils",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_EDITOR_EXPORTS",
		"GLORY_EDITOR_EXTENSION_EXPORTS",
		"GLORY_UUID_DEFINED",
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
