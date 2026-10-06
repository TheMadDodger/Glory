project "GloryVulkanGraphics"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"Module.yaml",
		"premake5.lua"
	}

	vpaths
	{
		["Module"] = { "GloryVulkan.*", "VulkanExceptions.h", "VulkanGraphicsModule.*", "VulkanStructsConverter.*", "VulkanDevice.*", "DescriptorAllocator.*", "CommandBufferAllocator.*" },
	}

	includedirs
	{
		"%{vulkanDir}/include",
		"%{vulkanDir}/third-party/include",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.VMA}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"vulkan-1",
		"GloryEngineCore",
		"GloryEngine",
		"yaml-cpp",

		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",
		"GloryECS",
	}

	defines
	{
		"GLORY_VULKAN_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkanDir}/Lib32",
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Lib",
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
