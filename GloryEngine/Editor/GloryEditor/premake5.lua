project "GloryEditor"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		["Backend"] = {  },
		["Backend/Base"] = { "EditorRenderImpl.*", "EditorWindowImpl.*" },
		["Extensions"] = { "BaseEditorExtension.*" },
		["Content"] = {  },
		["Content/Editors"] = { "GloryEditor.*", "GloryEditorVersion.*", "FileDialog.*", "EditorSettings.*", "BehaviorTreeEditor.*", "ComponentEditor.*", "Editor.*", "SceneObjectEditor.*", "ScriptableObjectEditor.*", "MaterialEditor.*", "MaterialInstanceEditor.*" },
		["Content/PropertyDrawers"] = { "SceneObjectPropertyDrawer.*", "StructPropertyDrawer.*", "EnumPropertyDrawer.*", "ArrayPropertyDrawer.*", "AssetReferencePropertyDrawer.*", "PropertyDrawer.*", "StandardPropertyDrawers.*" },
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.*", "FontTumbnailGenerator.*", "TextureTumbnailGenerator.*", "Tumbnail.*", "TumbnailGenerator.*", "SceneTumbnailGenerator.*" },
		["Editor"] = { "QuitPopup.*", "TitleBar.*", "Shortcuts.*", "EditorApplication.*", "EditorPlatform.*", "MainEditor.*", "MenuBar.*", "EditorAssets.*", "ProjectSpace.*", "Selection.*", "EditorCreateInfo.*", "Gizmos.*" },
		["Editor/ObjectMenu"] = { "ObjectMenu.*", "ObjectMenuCallbacks.*" },
		["Editor/Pipeline"] = { "EditorShaderProcessor.*", "EditorShaderData.*" },
		["Editor/Playmode"] = { "EditorPlayer.*", "IPlayModeHandler.*" },
		["Helpers"] = { "ImGuiHelpers.*" },
		["Popups"] = { "ObjectPicker.*", "AssetPicker.*", "PopupManager.*", "ProjectPopup.*", "AssetPickerPopup.*" },
		["Windows"] = { "ProjectSettingsWindow.*", "HistoryWindow.*", "EditorPreferencesWindow.*", "EditorWindow.*", "GameWindow.*", "InspectorWindow.*", "SceneGraphWindow.*", "EditorConsoleWindow.*" },
		["Windows/SceneView"] = { "SceneWindow.*", "SceneViewCamera.*" },
		["Windows/FileBrowser"] = { "FileBrowser.*", "FileBrowserItem.*" },
		["Windows/ProjectSettings"] = { "ProjectSettingsWindow.*" },
		["Windows/Analysis"] = { "PerformanceMetrics.*", "ProfilerWindow.*", "ProfilerTimeline.*" },
		["Undo"] = { "Undo.*" },
		["Undo/Actions"] = { "NodeValueChangeAction.*", "ValueChangeAction.*", "SetSiblingIndexAction.*", "SetParentAction.*", "ChangeObjectSceneAction.*", "DeleteSceneObjectAction.*", "SceneObjectNameAction.*", "SelectAction.*", "DeselectAction.*", "CreateObjectAction.*", "GizmoAction.*", "PropertyAction.*" },
		["Toolbar"] = { "Toolbar.*", "IToolChain.*", "GizmosToolChain.*" },
		["UI"] = { "EditorUI.*", "ListView.*" },
		["ProjectSettings"] = { "ProjectSettings.*", "ProjectSettings.*", "InputSettings.cpp", "EngineSettings.cpp", "LayerSettings.cpp" },
		["Asset Management"] = { "EditorAssetDatabase.*", "EditorAssetLoader.*", "EditorSceneManager.*", "EditorAssetCallbacks.*" }
	}

	includedirs
	{
		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ticpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.ImGui}",

		"%{IncludeDir.FA}",

		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
		"%{DepDirs.shaderc}",
		"%{LibDirs.spirv_cross}",

		"%{LibDirs.GloryECS}",
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

		"GloryReflectStatic",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_EDITOR_EXPORTS",
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