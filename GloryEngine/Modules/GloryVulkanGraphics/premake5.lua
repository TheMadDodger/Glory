project "GloryVulkanGraphics"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",

		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"vulkan-1",
		"GloryCore",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",

		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",
		"GloryECS",

		"GloryJobs",
		"GloryThreads",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
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
