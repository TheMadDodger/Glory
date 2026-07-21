project "FreeTypeFontImporter"
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
		"%{DepsIncludeDir}/freetype2",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.editor}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.FA}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.glory}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",

		"ImGui",
		"implot",
		"ImGuiTestEngine",
	}

	defines
	{
		"GLORY_FREETYPE_EXTENSION_EXPORTS"
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

		links "freetyped"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "freetype"
