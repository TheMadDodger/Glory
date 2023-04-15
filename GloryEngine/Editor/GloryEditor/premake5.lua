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
		["Content/PropertyDrawers"] = { "SceneObjectPropertyDrawer.*", "StructPropertyDrawer.*", "EnumPropertyDrawer.*", "ArrayPropertyDrawer.*", "AssetReferencePropertyDrawer.*", "PropertyDrawer.*", "StandardPropertyDrawers.*", "ShapePropertyDrawer.*" },
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.*", "FontTumbnailGenerator.*", "TextureTumbnailGenerator.*", "Tumbnail.*", "TumbnailGenerator.*", "SceneTumbnailGenerator.*" },
		["Editor"] = { "Configure.*", "QuitPopup.*", "TitleBar.*", "Shortcuts.*", "EditorApplication.*", "EditorPlatform.*", "MainEditor.*", "MenuBar.*", "EditorAssets.*", "ProjectSpace.*", "Selection.*", "EditorCreateInfo.*", "Gizmos.*" },
		["Editor/ObjectMenu"] = { "ObjectMenu.*", "ObjectMenuCallbacks.*" },
		["Editor/Pipeline"] = { "EditorShaderProcessor.*", "EditorShaderData.*" },
		["Editor/Playmode"] = { "EditorPlayer.*", "IPlayModeHandler.*" },
		["Helpers"] = { "ImGuiHelpers.*" },
		["Popups"] = { "VersionPopup.*", "ObjectPicker.*", "AssetPicker.*", "PopupManager.*", "ProjectPopup.*", "AssetPickerPopup.*" },
		["Windows"] = { "ProjectSettingsWindow.*", "HistoryWindow.*", "EditorPreferencesWindow.*", "EditorWindow.*", "GameWindow.*", "InspectorWindow.*", "SceneGraphWindow.*", "EditorConsoleWindow.*" },
		["Windows/SceneView"] = { "SceneWindow.*", "SceneViewCamera.*" },
		["Windows/FileBrowser"] = { "FileBrowser.*", "FileBrowserItem.*" },
		["Windows/ProjectSettings"] = { "ProjectSettingsWindow.*" },
		["Windows/Analysis"] = { "PerformanceMetrics.*", "ProfilerWindow.*", "ProfilerTimeline.*" },
		["Undo"] = { "Undo.*" },
		["Undo/Actions"] = { "YAMLAction.*", "NodeValueChangeAction.*", "ValueChangeAction.*", "SetSiblingIndexAction.*", "SetParentAction.*", "ChangeObjectSceneAction.*", "DeleteSceneObjectAction.*", "SceneObjectNameAction.*", "SelectAction.*", "DeselectAction.*", "CreateObjectAction.*", "GizmoAction.*", "PropertyAction.*" },
		["Toolbar"] = { "Toolbar.*", "IToolChain.*", "GizmosToolChain.*" },
		["UI"] = { "EditorUI.*", "ListView.*" },
		["ProjectSettings"] = { "ProjectSettings.*", "ProjectSettings.*", "InputSettings.cpp", "EngineSettings.cpp", "LayerSettings.cpp", "PhysicsSettings.cpp" },
		["Asset Management"] = { "JSONRef.*", "AssetCompiler.*", "EditorAssetDatabase.*", "EditorAssetLoader.*", "EditorSceneManager.*", "EditorAssetCallbacks.*" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.yaml_cpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.api}",

		"%{IncludeDir.FA}",

		"%{IncludeDir.Reflect}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"GloryAPI",
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