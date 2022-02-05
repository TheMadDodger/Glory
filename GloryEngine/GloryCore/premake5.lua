project "GloryCore"
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
		["Asset Management"] = { "AssetDatabase.*", "AssetGroup.*", "AssetLocation.*", "AssetManager.*", "ResourceMeta.*", "ResourceType.*" },
		["Console"] = { "Commands.*", "Console.*", "Debug.*", "DebugConsoleInput.*" },
		["Core"] = { "CoreExceptions.*", "Engine.*", "Game.*", "GameSettings.*", "GameState.*", "GloryCore.*", "GraphicsThread.*", "Object.*", "UUID.*" },
		["Job System"] = { "Job.*", "JobManager.*", "JobPool.*", "JobQueue.*" },
		["Modules"] = { "Module.*" },
		["Modules/Time"] = { "TimerModule.*", "GameTime.*" },
		["Modules/Graphics"] = { "GraphicsModule.*" },
		["Modules/Graphics/Data"] = { "GraphicsEnums.*", "GraphicsMemoryManager.*" },
		["Modules/Graphics/Geometry"] = { "VertexDefinitions.*", "VertexHelpers.*" },
		["Modules/Graphics/Rendering"] = { "FrameState.*", "RenderFrame.*", "RenderQueue.*" },
		["Modules/Graphics/Resources"] = { "Buffer.*", "GPUResource.*", "GPUResourceManager.*", "Material.*", "Mesh.*", "Shader.*", "Texture.*", "RenderTexture.*" },
		["Modules/Renderer"] = { "RenderData.*", "RendererModule.*", "FrameStates.*" },
		["Modules/Renderer/Camera"] = { "Camera.*", "CameraManager.*", "DisplayManager.*", "CameraRef.*" },
		["Modules/Renderer/Layers"] = { "Layer.*", "LayerManager.*", "LayerMask.*" },
		["Modules/Renderer/Lighting"] = { "LightData.*" },
		["Modules/ResourceLoading"] = {  },
		["Modules/ResourceLoading/Base"] = { "ImportSettings.*", "Resource.*", "ResourceLoaderModule.*" },
		["Modules/ResourceLoading/File"] = { "FileData.*", "FileLoaderModule.*" },
		["Modules/ResourceLoading/Material"] = { "MaterialData.*", "MaterialPropertyData.*", "MaterialInstanceData.*" },
		["Modules/ResourceLoading/Models"] = { "MeshData.*", "ModelData.*", "ModelLoaderModule.*" },
		["Modules/ResourceLoading/Shaders"] = { "ShaderCrossCompiler.*", "ShaderData.*", "ShaderLoaderModule.*" },
		["Modules/ResourceLoading/Textures"] = { "ImageData.*", "ImageLoaderModule.*" },
		["Modules/Scenes"] = { "ScenesModule.*", "SceneObject.*", "GScene.*" },
		["Modules/Window"] = { "Window.*", "WindowModule.*" },
		["Threading"] = { "Thread.*", "ThreadManager.*" },
		["Analysis"] = { "EngineProfiler.*", "ProfilerModule.*", "ProfilerSample.*", "ProfilerThreadSample.*" },
		["Helpers"] = { "GLORY_YAML.*", "YAML_GLM.*" },
		["Serialization"] = { "PropertyFlags.*", "PropertySerializer.*", "SerializedProperty.*", "Serializer.*", "AssetReferencePropertySerializer.*" },
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}"
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
