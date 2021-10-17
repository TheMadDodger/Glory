project "GloryVulkanGraphics"
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
		"%{GloryIncludeDir.core}"
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