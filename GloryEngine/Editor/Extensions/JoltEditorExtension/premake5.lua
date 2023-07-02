project "JoltEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
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

		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.jolt}",

		"%{IncludeDir.FA}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",

		"%{rootDir}/submodules/JoltPhysics",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.implot}",
		"%{LibDirs.glory}",
		"%{modulesDir}/GloryJoltPhysics",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"GloryEditor",
		"GloryJoltPhysics",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",

		"GloryECS",
		"GloryReflect"
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
