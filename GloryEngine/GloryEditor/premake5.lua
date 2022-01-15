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
		["Backend/Base"] = { "EditorRenderImpl.h", "EditorRenderImpl.cpp", "EditorWindowImpl.h", "EditorWindowImpl.cpp" },
		["Backend/OpenGL"] = { "EditorOpenGLRenderImpl.h", "EditorOpenGLRenderImpl.cpp" },
		["Extensions"] = { "BaseEditorExtension.h", "BaseEditorExtension.cpp" },
		["Backend/SDL"] = { "EditorSDLWindowImpl.h", "EditorSDLWindowImpl.cpp" },
		["Backend/Vulkan"] = { "EditorVulkanRenderImpl.h", "EditorVulkanRenderImpl.cpp" },
		["Content"] = {  },
		["Content/Editors"] = { "BehaviorTreeEditor.h", "BehaviorTreeEditor.cpp", "ComponentEditor.h", "ComponentEditor.cpp", "Editor.h", "Editor.cpp", "SceneObjectEditor.h", "SceneObjectEditor.cpp", "ScriptableObjectEditor.h", "ScriptableObjectEditor.cpp" },
		["Content/PropertyDrawers"] = { "AssetReferencePropertyDrawer.h", "AssetReferencePropertyDrawer.cpp", "PropertyDrawer.h", "PropertyDrawer.cpp", "StandardPropertyDrawers.h", "StandardPropertyDrawers.cpp" },
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.h", "AudioTumbnailGenerator.cpp", "FontTumbnailGenerator.h", "FontTumbnailGenerator.cpp", "TextureTumbnailGenerator.h", "TextureTumbnailGenerator.cpp", "Tumbnail.h", "Tumbnail.cpp", "TumbnailGenerator.h", "TumbnailGenerator.cpp" },
		["Editor"] = { "EditorApplication.h", "EditorApplication.cpp", "EditorPlatform.h", "EditorPlatform.cpp", "MainEditor.h", "MainEditor.cpp", "MenuBar.h", "MenuBar.cpp", "EditorAssets.h", "EditorAssets.cpp", "EditorAssetsLoader.h", "EditorAssetsLoader.cpp", "ProjectSpace.h", "ProjectSpace.cpp", "EditorAssetLoader.h", "EditorAssetLoader.cpp", "Selection.h", "Selection.cpp", "EditorCreateInfo.h", "EditorCreateInfo.cpp" },
		["ImGui"] = { "imgui_impl_opengl3.h", "imgui_impl_opengl3.cpp", "imgui_impl_sdl.h", "imgui_impl_sdl.cpp", "imgui_impl_vulkan.h", "imgui_impl_vulkan.cpp" },
		["OLD"] = { "AssetPickerPopup.h", "AssetPickerPopup.cpp", "ComponentPopup.h", "ComponentPopup.cpp", "EditorSceneManager.h", "EditorSceneManager.cpp", "LevelEditor.h", "LevelEditor.cpp", "SkeletonEditorComponent.h", "SkeletonEditorComponent.cpp", "SkeletonObject.h", "SkeletonObject.cpp" },
		["Popups"] = { "PopupManager.h", "PopupManager.cpp", "ProjectPopup.h", "ProjectPopup.cpp" },
		["Windows"] = { "EditorPreferencesWindow.h", "EditorPreferencesWindow.cpp", "EditorWindow.h", "EditorWindow.cpp", "GameWindow.h", "GameWindow.cpp", "InspectorWindow.h", "InspectorWindow.cpp", "SceneGraphWindow.h", "SceneGraphWindow.cpp" },
		["Windows/SceneView"] = { "SceneWindow.h", "SceneWindow.cpp", "SceneViewCamera.h", "SceneViewCamera.cpp" },
		["Windows/ContentBrowser"] = { "ContentBrowser.h", "ContentBrowser.cpp", "ContentBrowserItem.h", "ContentBrowserItem.cpp" }
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
		"%{IncludeDir.SDL_Image}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.assimp}",
		"%{GloryIncludeDir.entityscenes}",
		"%{GloryIncludeDir.basicrenderer}",
		"%{GloryIncludeDir.glslloader}",
		"%{GloryIncludeDir.opengl}",
		"%{GloryIncludeDir.sdlimage}",
		"%{GloryIncludeDir.sdlwindow}",
		"%{GloryIncludeDir.vulkan}",
		"%{GloryIncludeDir.ImGui}"
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
