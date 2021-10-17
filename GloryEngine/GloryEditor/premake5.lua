project "GloryEditor"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Backend"] = {  }
		["Backend/Base"] = { "EditorRenderImpl.h", "EditorRenderImpl.cpp", "EditorWindowImpl.h", "EditorWindowImpl.cpp" }
		["Backend/OpenGL"] = { "EditorOpenGLRenderImpl.h", "EditorOpenGLRenderImpl.cpp" }
		["Backend/SDL"] = { "EditorSDLWindowImpl.h", "EditorSDLWindowImpl.cpp" }
		["Backend/Vulkan"] = { "EditorVulkanRenderImpl.h", "EditorVulkanRenderImpl.cpp" }
		["Content"] = {  }
		["Content/Editors"] = { "BehaviorTreeEditor.h", "BehaviorTreeEditor.cpp", "ComponentEditor.h", "ComponentEditor.cpp", "Editor.h", "Editor.cpp", "GameObjectEditor.h", "GameObjectEditor.h", "ScriptableObjectEditor.h", "ScriptableObjectEditor.cpp" }
		["Content/PropertyDrawers"] = { "AssetReferencePropertyDrawer.h", "AssetReferencePropertyDrawer.cpp", "PropertyDrawer.h", "PropertyDrawer.cpp", "StandardPropertyDrawers.h", "StandardPropertyDrawers.cpp" }
		["Content/Tumbnails"] = { "AudioTumbnailGenerator.h", "AudioTumbnailGenerator.cpp", "FontTumbnailGenerator.h", "FontTumbnailGenerator.cpp", "TextureTumbnailGenerator.h", "TextureTumbnailGenerator.cpp", "Tumbnail.h", "Tumbnail.cpp", "TumbnailGenerator.h", "TumbnailGenerator.cpp" }
		["Editor"] = { "EditorApplication.h", "EditorApplication.cpp", "EditorPlatform.h", "EditorPlatform.cpp", "MainEditor.h", "MainEditor.cpp", "MenuBar.h", "MenuBar.cpp" }
		["ImGui"] = { "imgui_impl_opengl3.h", "imgui_impl_opengl3.cpp", "imgui_impl_sdl.h", "imgui_impl_sdl.cpp", "imgui_impl_vulkan.h", "imgui_impl_vulkan.cpp" }
		["OLD"] = { "AssetPickerPopup.h", "AssetPickerPopup.cpp", "ComponentPopup.h", "ComponentPopup.cpp", "EditorSceneManager.h", "EditorSceneManager.cpp", "LevelEditor.h", "LevelEditor.cpp", "SceneViewCamera.h", "SceneViewCamera.cpp", "Selection.h", "Selection.cpp", "SkeletonEditorComponent.h", "SkeletonEditorComponent.cpp", "SkeletonObject.h", "SkeletonObject.cpp" }
		["Popups"] = { "PopupManager.h", "PopupManager.cpp" }
		["Windows"] = { "ContentBrowser.h", "ContentBrowser.cpp", "EditorPreferencesWindow.h", "EditorPreferencesWindow.cpp", "EditorWindow.h", "EditorWindow.cpp", "GameWindow.h", "GameWindow.cpp", "InspectorWindow.h", "InspectorWindow.cpp", "SceneGraphWindow.h", "SceneGraphWindow.cpp", "SceneWindow.h", "SceneWindow.cpp" }
	}

	includedirs
	{
		"%(vulkan_sdk)\third-party\include",
		"$(SolutionDir)bin\assimp\include"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v142"

		defines
		{
			"_LIB"
		}

	filter "platforms:x86"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

