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
		["Module"] = { "Components.h", "Components.cpp", "Entity.h", "Entity.cpp", "EntityScene.h", "EntityScene.cpp", "EntitySceneScenesModule.h", "EntitySceneScenesModule.cpp" },
		["Systems"] = { "EntitySystem.h", "EntitySystem.cpp", "EntitySystems.h", "EntitySystems.cpp", "MeshRenderSystem.h", "MeshRenderSystem.cpp", "Systems.h", "TransformSystem.h", "TransformSystem.cpp" }
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{vulkan_sdk}/third-party/include"
	}

	filter "system:windows"
		systemversion "latest"
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
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter {"system:windows", "configurations:Release" }
		buildoptions "/MDd"

	filter {"system:windows", "configurations:Release" }
		buildoptions "/MD"