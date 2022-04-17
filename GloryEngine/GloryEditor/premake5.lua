project "GloryEditor"
	kind "StaticLib"
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
		["Backend/OpenGL"] = { "EditorOpenGLRenderImpl.*" },
		["Extensions"] = { "BaseEditorExtension.*" },
		["Backend/SDL"] = { "EditorSDLWindowImpl.*" },
		["Backend/Vulkan"] = { "EditorVulkanRenderImpl.*" },
		["Content"] = {  },
		["Content/Editors"] = { "BehaviorTreeEditor.*", "ComponentEditor.*", "Editor.*", "SceneObjectEditor.*", "ScriptableObjectEditor.*", "MaterialEditor.*", "MaterialInstanceEditor.*" },
		["Content/PropertyDrawers"] = { "AssetReferencePropertyDrawer.*", "PropertyDrawer.*", "StandardPropertyDrawers.*" },
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.*", "FontTumbnailGenerator.*", "TextureTumbnailGenerator.*", "Tumbnail.*", "TumbnailGenerator.*", "SceneTumbnailGenerator.*" },
		["Editor"] = { "EditorApplication.*", "EditorPlatform.*", "MainEditor.*", "MenuBar.*", "EditorAssets.*", "EditorAssetsLoader.*", "ProjectSpace.*", "EditorAssetLoader.*", "Selection.*", "EditorCreateInfo.*", "EditorSceneManager.*", "Gizmos.*" },
		["Editor/ObjectMenu"] = { "ObjectMenu.*", "ObjectMenuCallbacks.*" },
		["Editor/Pipeline"] = { "EditorShaderProcessor.*", "EditorShaderData.*" },
		["ImGui"] = { "imgui_impl_opengl3.*", "imgui_impl_sdl.*", "imgui_impl_vulkan.*" },
		["Helpers"] = { "ImGuiHelpers.*" },
		["Popups"] = { "PopupManager.*", "ProjectPopup.*", "AssetPickerPopup.*" },
		["Windows"] = { "EditorPreferencesWindow.*", "EditorWindow.*", "GameWindow.*", "InspectorWindow.*", "SceneGraphWindow.*", "EditorConsoleWindow.*" },
		["Windows/SceneView"] = { "SceneWindow.*", "SceneViewCamera.*" },
		["Windows/ContentBrowser"] = { "ContentBrowser.*", "ContentBrowserItem.*" },
		["Windows/Analysis"] = { "PerformanceMetrics.*", "ProfilerWindow.*", "ProfilerTimeline.*" }
	}

	includedirs
	{
		"%{vulkan_sdk}/include",
		"%{vulkan_sdk}/third-party/include",

		"%{IncludeDir.assimp}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.SDL_Image}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ticpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.assimp}",
		"%{GloryIncludeDir.entityscenes}",
		"%{GloryIncludeDir.basicrenderer}",
		"%{GloryIncludeDir.opengl}",
		"%{GloryIncludeDir.sdlimage}",
		"%{GloryIncludeDir.sdlwindow}",
		"%{GloryIncludeDir.vulkan}",
		"%{GloryIncludeDir.ImGui}",
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
