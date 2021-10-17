project "GloryCore"
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
		["Asset Management"] = { "AssetDatabase.h", "AssetDatabase.cpp", "AssetGroup.h", "AssetGroup.cpp", "AssetLocation.h", "AssetLocation.cpp", "AssetManager.h", "AssetManager.cpp" }
		["Console"] = { "Commands.h", "Commands.cpp", "Console.h", "Console.cpp", "Debug.h", "Debug.cpp", "DebugConsoleInput.h", "DebugConsoleInput.cpp" }
		["Core"] = { "CoreExceptions.h", "Engine.h", "Engine.cpp", "Game.h", "Game.cpp", "GameSettings.h", "GameState.h", "GameState.cpp", "GameThread.h", "GameThread.cpp", "GloryCore.h", "GraphicsThread.h", "GraphicsThread.cpp", "MainThread.h", "MainThread.cpp", "Object.h", "Object.cpp", "UUID.h", "UUID.cpp" }
		["Job System"] = { "Job.h", "Job.cpp", "JobManager.h", "JobManager.cpp", "JobPool.h", "JobPool.cpp", "JobQueue.h", "JobQueue.cpp" }
		["Modules"] = { "Module.h", "Module.cpp" }
		["Modules/Graphics"] = { "GraphicsModule.h", "GraphicsModule.cpp" }
		["Modules/Graphics/Data"] = { "GraphicsEnums.h", "GraphicsMemoryManager.h", "GraphicsMemoryManager.cpp" }
		["Modules/Graphics/Geometry"] = { "VertexDefinitions.h", "VertexHelpers.h" }
		["Modules/Graphics/Rendering"] = { "FrameState.h", "FrameState.cpp", "RenderFrame.h", "RenderFrame.cpp", "RenderQueue.h", "RenderQueue.cpp" }
		["Modules/Graphics/Resources"] = { "Buffer.h", "Buffer.cpp", "GPUResource.h", "GPUResource.cpp", "GPUResourceManager.h", "GPUResourceManager.cpp", "Material.h", "Material.cpp", "Mesh.h", "Mesh.cpp", "Shader.h", "Shader.cpp", "Texture.h", "Texture.cpp" }
		["Modules/Renderer"] = { "RenderData.h", "RenderModule.h", "RenderModule.cpp" }
		["Modules/ResourceLoading"] = {  }
		["Modules/ResourceLoading/Base"] = { "ImportSettings.h", "ImportSettings.cpp", "Resource.h", "Resource.cpp", "ResourceLoaderModule.h", "ResourceLoaderModule.cpp" }
		["Modules/ResourceLoading/File"] = { "FileData.h", "FileData.cpp", "FileLoaderModule.h", "FileLoaderModule.cpp" }
		["Modules/ResourceLoading/Material"] = { "MaterialData.h", "MaterialData.cpp", "MaterialPropertyData.h", "MaterialPropertyData.cpp" }
		["Modules/ResourceLoading/Models"] = { "MeshData.h", "MeshData.cpp", "ModelData.h", "ModelData.cpp", "ModelLoaderModule.h", "ModelLoaderModule.cpp" }
		["Modules/ResourceLoading/Shaders"] = { "ShaderCrossCompiler.h", "ShaderCrossCompiler.cpp", "ShaderData.h", "ShaderData.cpp", "ShaderLoaderModule.h", "ShaderLoaderModule.cpp" }
		["Modules/ResourceLoading/Textures"] = { "ImageData.h", "ImageData.cpp" }
		["Modules/Scenes"] = { "ScenesModule.h", "ScenesModule.cpp" }
		["Modules/Window"] = { "Window.h", "Window.cpp", "WindowModule.h", "WindowModule.cpp" }
		["Threading"] = { "Thread.h", "Thread.cpp", "ThreadManager.h", "ThreadManager.cpp" }
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

