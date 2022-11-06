project "EntityScenesEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryEntityScenes/Editor/Extension")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Extension"] = { "EntityScenesEditorExtension.*" },
		["Editors"] = { "TransformEditor.*", "EntitySceneObjectEditor.*", "DefaultComponentEditor.*", "EntityComponentEditor.*", "CameraComponentEditor.*", "EntityComponentPopup.*", "LayerComponentEditor.*", "MeshRendererComponentEditor.*" },
		["Actions"] = { "AddComponentAction.*", "RemoveComponentAction.*" }
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.entityscenes}",
		"%{GloryIncludeDir.ImGui}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}"
	}

	libdirs
	{
		"%{LibDirs.ImGui}",
		"%{LibDirs.LibDirs}",
		"%{LibDirs.implot}",
		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GloryEntityScenes",
		"%{LibDirs.assimp}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"GloryEditor",
		"GloryEntityScenes",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",

		"GloryECSStatic",
		"GloryReflectStatic"
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_UUID_DEFINED",
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v142"

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
