project "TestEngineExtension"
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

		"%{GloryIncludeDir.serialization}",
		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.ui}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuiTestEngine}",
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
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryEditorSerialization",
		"GloryUIRenderer",
		"GloryECS",
		"GloryReflect",
		"GloryUtils",
		"GloryUtilsVersion",

		"ImGui",
		"implot",
		"ImGuiTestEngine",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_TESTENGINE_EXTENSION_EXPORTS"
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
