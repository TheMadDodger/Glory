project "GloryEditor"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Backend"] = {  },
		["Backend/Base"] = { "EditorRenderImpl.*", "EditorWindowImpl.*" },
		["Extensions"] = { "BaseEditorExtension.*" },
		["Content"] = {  },
		["Content/Editors"] = { "GloryEditor.*", "GloryEditorVersion.*", "FileDialog.*", "EditorSettings.*", "BehaviorTreeEditor.*", "ComponentEditor.*", "Editor.*", "SceneObjectEditor.*", "ScriptableObjectEditor.*", "MaterialEditor.*", "MaterialInstanceEditor.*" },
		["Content/PropertyDrawers"] = { "ArrayPropertyDrawer.*", "AssetReferencePropertyDrawer.*", "PropertyDrawer.*", "StandardPropertyDrawers.*" },
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.*", "FontTumbnailGenerator.*", "TextureTumbnailGenerator.*", "Tumbnail.*", "TumbnailGenerator.*", "SceneTumbnailGenerator.*" },
		["Editor"] = { "EditorApplication.*", "EditorPlatform.*", "MainEditor.*", "MenuBar.*", "EditorAssets.*", "EditorAssetsLoader.*", "ProjectSpace.*", "EditorAssetLoader.*", "Selection.*", "EditorCreateInfo.*", "EditorSceneManager.*", "Gizmos.*" },
		["Editor/ObjectMenu"] = { "ObjectMenu.*", "ObjectMenuCallbacks.*" },
		["Editor/Pipeline"] = { "EditorShaderProcessor.*", "EditorShaderData.*" },
		["Editor/Playmode"] = { "EditorPlayer.*", "IEditorLoopHandler.*" },
		["Helpers"] = { "ImGuiHelpers.*" },
		["Popups"] = { "PopupManager.*", "ProjectPopup.*", "AssetPickerPopup.*" },
		["Windows"] = { "EditorPreferencesWindow.*", "EditorWindow.*", "GameWindow.*", "InspectorWindow.*", "SceneGraphWindow.*", "EditorConsoleWindow.*" },
		["Windows/SceneView"] = { "SceneWindow.*", "SceneViewCamera.*" },
		["Windows/ContentBrowser"] = { "ContentBrowser.*", "ContentBrowserItem.*" },
		["Windows/Analysis"] = { "PerformanceMetrics.*", "ProfilerWindow.*", "ProfilerTimeline.*" },
		["Undo"] = { "Undo.*" },
		["Undo/Actions"] = { "ChangeObjectSceneAction.*", "DeleteSceneObjectAction.*", "SceneObjectNameAction.*", "SelectAction.*", "DeselectAction.*", "CreateObjectAction.*", "GizmoAction.*", "PropertyAction.*" },
		["Toolbar"] = { "Toolbar.*", "IToolChain.*" }
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",
		
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.ImGui}",
	}
	
	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
	}
	
	links
	{
		"GloryCore",
		"yaml-cpp",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"ImFileDialog",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_EDITOR_EXPORTS"
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
		
		links
		{
			"spirv-cross-cd",
			"spirv-cross-cored",
			"spirv-cross-cppd",
			"spirv-cross-glsld",
			"spirv-cross-hlsld",
			"spirv-cross-msld",
			"spirv-cross-reflectd",
			"spirv-cross-utild",
		}

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
		
		links
		{
			"spirv-cross-c",
			"spirv-cross-core",
			"spirv-cross-cpp",
			"spirv-cross-glsl",
			"spirv-cross-hlsl",
			"spirv-cross-msl",
			"spirv-cross-reflect",
			"spirv-cross-util",
		}