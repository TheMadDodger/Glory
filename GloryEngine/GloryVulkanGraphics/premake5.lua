project "GloryVulkanGraphics"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp",
		"Module.yaml"
	}

	vpaths
	{
		["CommandHandlers"] = { "PipelineCommandHandlers.h", "PipelineCommandHandlers.cpp", "RenderPassCommandHandlers.h", "RenderPassCommandHandlers.cpp" },
		["Device"] = { "Device.h", "Device.cpp", "QueueFamilyIndices.h", "VulkanDeviceManager.h", "VulkanDeviceManager.cpp" },
		["Graphics"] = { "DeferredPipelineTest.h", "DeferredPipelineTest.cpp", "DeferredRenderPassTest.h", "DeferredRenderPassTest.cpp", "DepthImage.h", "DepthImage.cpp", "SwapChain.h", "SwapChain.cpp", "VulkanGraphicsPipeline.h", "VulkanGraphicsPipeline.cpp", "VulkanImageSampler.h", "VulkanImageSampler.cpp", "VulkanRenderPass.h", "VulkanRenderPass.cpp" },
		["Module"] = { "GloryVulkan.h", "VulkanExceptions.h", "VulkanGraphicsModule.h", "VulkanGraphicsModule.cpp", "VulkanStructsConverter.h", "VulkanStructsConverter.cpp" },
		["Resources"] = { "VulkanBuffer.h", "VulkanBuffer.cpp", "VulkanMesh.h", "VulkanMesh.cpp", "VulkanResourceManager.h", "VulkanResourceManager.cpp", "VulkanShader.h", "VulkanShader.cpp", "VulkanTexture.h", "VulkanTexture.cpp" }
	}

	includedirs
	{
		"%{vulkan_sdk}/include",
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}"
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"vulkan-1",
		"GloryCore",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_EXPORTS"
	}
	
	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
		("{COPY} ./Assets %{moduleOutDir}/Assets"),
		("{COPY} ./Resources %{moduleOutDir}/Resources"),
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

		libdirs
		{
			"%{vulkan_sdk}/Lib32",
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkan_sdk}/Lib",
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
