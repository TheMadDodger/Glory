project "JoltEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryJoltPhysics/Editor/Extension")
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

		"%{BaseIncludeDir.physics}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.serialization}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.jolt}",
		"%{GloryIncludeDir.threads}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.FA}",

		"%{rootDir}/submodules/JoltPhysics",

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

		"%{modulesDir}/GloryJoltPhysics",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryEditorSerialization",
		"GloryJoltPhysics",

		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",

		"ImGui",
		"ImGuiTestEngine",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_JOLT_EXTENSION_EXPORTS",
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

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
