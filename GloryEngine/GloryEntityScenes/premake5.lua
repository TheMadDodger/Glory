project "GloryEntityScenes"
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
		["ECS"] = { "EntityComponentData.h", "EntityID.h", "Registry.h", "Registry.cpp" },
		["Module"] = { "Components.h", "Components.cpp", "Entity.h", "Entity.cpp", "EntityScene.h", "EntityScene.cpp", "EntitySceneScenesModule.h", "EntitySceneScenesModule.cpp", "EntityComponentObject.h", "EntityComponentObject.cpp", "EntitySceneObject.h", "EntitySceneObject.cpp" },
		["Serializers"] = { "EntitySceneSerializer.h", "EntitySceneSerializer.cpp", "EntitySceneObjectSerializer.h", "EntitySceneObjectSerializer.cpp", "EntityComponentSerializer.h", "TransformSerializer.h", "TransformSerializer.cpp" },
		["Systems"] = { "EntitySystem.h", "EntitySystem.cpp", "EntitySystems.h", "EntitySystems.cpp", "MeshRenderSystem.h", "MeshRenderSystem.cpp", "Systems.h", "TransformSystem.h", "TransformSystem.cpp", "CameraSystem.h", "CameraSystem.cpp", "LookAtSystem.h", "LookAtSystem.cpp", "SpinSystem.h", "SpinSystem.cpp" },
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.yaml_cpp}",
		"%{vulkan_sdk}/third-party/include"
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
