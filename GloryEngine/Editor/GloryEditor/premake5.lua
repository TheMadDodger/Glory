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
		["Editor"] = { "EntityEditor.*", "EditableEntity.*", "Dispatcher.h", "DND.*", "Configure.*", "QuitPopup.*", "TitleBar.*", "Shortcuts.*", "EditorApplication.*", "EditorPlatform.*", "MainEditor.*", "MenuBar.*", "EditorAssets.*", "ProjectSpace.*", "Selection.*", "EditorCreateInfo.*", "Gizmos.*", "RemovedAssetsPopup.*" },
		["Editor/ObjectMenu"] = { "ObjectMenu.*", "ObjectMenuCallbacks.*", "CreateEntityObjectsCallbacks.*" },
		["Editor/Pipeline"] = { "EditorShaderData.*" },
		["Editor/Playmode"] = { "EditorPlayer.*", "IPlayModeHandler.*" },
		["Editor/Tumbnails"] = { "ImageTumbnailGenerator.*", "AudioTumbnailGenerator.*", "FontTumbnailGenerator.*", "TextureTumbnailGenerator.*", "Tumbnail.*", "TumbnailGenerator.*", "SceneTumbnailGenerator.*" },
		["Editor/PropertyDrawers"] = { "MeshMaterialPropertyDrawer.*", "SceneObjectPropertyDrawer.*", "StructPropertyDrawer.*", "EnumPropertyDrawer.*", "ArrayPropertyDrawer.*", "AssetReferencePropertyDrawer.*", "PropertyDrawer.*", "StandardPropertyDrawers.*", "ShapePropertyDrawer.*" },
		["Editor/Editors"] = { "TextureDataEditor.*", "GloryEditor.*", "GloryEditorVersion.*", "FileDialog.*", "EditorSettings.*", "BehaviorTreeEditor.*", "ComponentEditor.*", "Editor.*", "SceneObjectEditor.*", "ScriptableObjectEditor.*", "MaterialEditor.*", "MaterialInstanceEditor.*" },
		["Editor/Editors/Component"] = { "TransformEditor.*", "DefaultComponentEditor.*", "EntityComponentEditor.*", "EntityComponentPopup.*", "EntitySceneObjectEditor.*", "LightComponentEditor.*" },
		["Helpers"] = { "ImGuiHelpers.*" },
		["Popups"] = { "VersionPopup.*", "ObjectPicker.*", "AssetPicker.*", "PopupManager.*", "ProjectPopup.*", "AssetPickerPopup.*" },
		["Windows"] = { "ResourcesWindow.*", "ProjectSettingsWindow.*", "HistoryWindow.*", "EditorPreferencesWindow.*", "EditorWindow.*", "GameWindow.*", "InspectorWindow.*", "SceneGraphWindow.*", "EditorConsoleWindow.*" },
		["Windows/SceneView"] = { "SceneWindow.*", "SceneViewCamera.*" },
		["Windows/FileBrowser"] = { "FileBrowser.*", "FileBrowserItem.*" },
		["Windows/ProjectSettings"] = { "ProjectSettingsWindow.*" },
		["Windows/Analysis"] = { "PerformanceMetrics.*", "ProfilerWindow.*", "ProfilerTimeline.*" },
		["Undo"] = { "Undo.*" },
		["Undo/Actions"] = { "EnableObjectAction.*", "YAMLAction.*", "NodeValueChangeAction.*", "ValueChangeAction.*", "SetSiblingIndexAction.*", "SetParentAction.*", "ChangeObjectSceneAction.*", "DeleteSceneObjectAction.*", "SceneObjectNameAction.*", "SelectionChangedAction.*", "CreateObjectAction.*", "GizmoAction.*", "PropertyAction.*" },
		["Undo/Actions/Entity"] = { "AddComponentAction.*", "RemoveComponentAction.*" },
		["Toolbar"] = { "Toolbar.*", "IToolChain.*", "GizmosToolChain.*", "GridToolchain.*" },
		["UI"] = { "EditorUI.*", "ListView.*" },
		["ProjectSettings"] = { "ProjectSettings.*", "ProjectSettings.*", "InputSettings.cpp", "EngineSettings.cpp", "LayerSettings.cpp", "PhysicsSettings.cpp" },
		["Asset Management"] = { "EditorSceneSerializer*", "EditorAssetsWatcher.*", "JSONRef.*", "AssetCompiler.*", "EditorAssetDatabase.*", "EditorAssetLoader.*", "EditorSceneManager.*", "EditorAssetCallbacks.*" },
		["Asset Management/Importer"] = { "Importer.*", "ImporterTemplate.*", "ImportedResource.*" },
		["Asset Management/Importer/Internal"] = { "MaterialImporter.*", "MaterialInstanceImporter.*", "TextureImporter.*", "EntityPrefabImporter.*" },
		["Migration"] = { "ProjectMigrations.*" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.yaml_cpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.api}",

		"%{IncludeDir.FA}",

		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",

		"%{rapidjson}",

		"%{rootDir}/third-party/tinyfiledialogs"
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"GloryJobs",
		"GloryThreads",
		"GloryAPI",
		"yaml-cpp",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",

		"GloryReflect",
		"GloryUtils",
		"GloryECS",
		"GloryUtilsVersion",
		"tinyfiledialogs",
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
			"efsw-static-debug"
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
			"efsw-static-release"
		}