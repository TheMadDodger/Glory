project "GloryEditorSDL"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GlorySDLWindow/Editor/Backend")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_sdl2.h",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_sdl2.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{DepsIncludeDir}",
		"%{DepsIncludeDir}/SDL2",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.sdlwindow}",
		"%{GloryIncludeDir.threads}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.FA}",

		"%{SubmodoleDirs.ImGui}/backends/",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GlorySDLWindow",

		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuiTestEngine}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryReflect",
		"GloryECS",
		"GlorySDLWindow",
		"GloryEditor",
		"GloryUtilsVersion",
		"GloryUtils",

		"SDL2",
		"yaml-cpp",
		"ImGui",
		"ImGuiTestEngine",
	}

	defines
	{
		"GLORY_EDITORSDL_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
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
