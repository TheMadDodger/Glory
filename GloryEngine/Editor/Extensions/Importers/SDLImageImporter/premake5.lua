project "SDLImageImporter"
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

		"%{IncludeDir.FA}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
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
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"SDL2",
		"SDL2_image",

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
		"GLORY_SDL_IMAGE_EXTENSION_EXPORTS"
	}

	postbuildcommands
	{
		("{COPY} %{DepsBinDir}/SDL2_image*.dll %{engineOutDir}/Extensions/Dependencies"),
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
