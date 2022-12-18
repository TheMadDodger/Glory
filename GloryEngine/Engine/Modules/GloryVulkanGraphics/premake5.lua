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
		["CommandHandlers"] = { "PipelineCommandHandlers.h", "PipelineCommandHandlers.cpp", "RenderPassCommandHandlers.h", "RenderPassCommandHandlers.cpp" },
		["Device"] = { "Device.h", "Device.cpp", "QueueFamilyIndices.h", "VulkanDeviceManager.h", "VulkanDeviceManager.cpp" },
		["Graphics"] = { "DeferredPipelineTest.h", "DeferredPipelineTest.cpp", "DeferredRenderPassTest.h", "DeferredRenderPassTest.cpp", "DepthImage.h", "DepthImage.cpp", "SwapChain.h", "SwapChain.cpp", "VulkanGraphicsPipeline.h", "VulkanGraphicsPipeline.cpp", "VulkanImageSampler.h", "VulkanImageSampler.cpp", "VulkanRenderPass.h", "VulkanRenderPass.cpp" },
		["Module"] = { "GloryVulkan.h", "VulkanExceptions.h", "VulkanGraphicsModule.h", "VulkanGraphicsModule.cpp", "VulkanStructsConverter.h", "VulkanStructsConverter.cpp" },
		["Resources"] = { "VulkanBuffer.h", "VulkanBuffer.cpp", "VulkanMesh.h", "VulkanMesh.cpp", "VulkanResourceManager.h", "VulkanResourceManager.cpp", "VulkanShader.h", "VulkanShader.cpp", "VulkanTexture.h", "VulkanTexture.cpp" }
	}

	includedirs
	{
		"%{vulkanDir}/include",
		"%{vulkanDir}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		
		"%{IncludeDir.Reflect}",
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
		
		"GloryReflectStatic",
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
