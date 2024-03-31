project "GloryCore"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		["Asset Management"] = { "PrefabData.*", "BinaryStream.*", "AssetArchive.*", "AssetDatabase.*", "AssetGroup.*", "AssetLocation.*", "AssetManager.*", "ResourceMeta.*", "ResourceType.*", "ShaderManager.*", "AssetCallbacks.*" },
		["Resources"] = { "PrefabData.*", "MaterialData.*", "MaterialPropertyData.*", "MaterialInstanceData.*", "MaterialPropertyInfo.*", "TextureData.*", "MeshData.*", "ModelData.*", "ImageData.*", "FileData.*", "ShaderSourceData.*" },
		["Console"] = { "Logs.*", "CommandLine.*", "Commands.*", "Console.*", "Debug.*", "DebugConsoleInput.*", "IConsole.*", "WindowsDebugConsole.*" },
		["Core"] = { "ObjectManager.*", "TypeFlags.*", "GloryContext.*", "BuiltInModules.*", "Engine.*", "GraphicsThread.*", "Object.*", "UUID.*", "Glory.*" },
		["Modules/Base"] = { "Module.*", "ModuleMetaData.*", "IModuleLoopHandler.*", "ModuleSettings.*" },
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
		["Modules/ResourceLoading/File"] = { "FileLoaderModule.*" },
		["Modules/ResourceLoading/Models"] = { "ModelLoaderModule.*" },
		["Modules/ResourceLoading/Shaders"] = { "ShaderCrossCompiler.*" },
		["Modules/ResourceLoading/Textures"] = { "TextureDataLoaderModule.*" },
		["Modules/Window"] = { "Window.*", "WindowModule.*" },
		["Modules/Input"] = { "InputModule.*", "Input.*", "KeyEnums.*", "PlayerInput.*" },
		["Modules/Physics"] = { "ShapeManager.*", "CharacterManager.*", "PhysicsModule.*", "Shapes.*", "Physics.*", "ShapeProperty.*" },
		["Analysis"] = { "EngineProfiler.*", "ProfilerModule.*", "ProfilerSample.*", "ProfilerThreadSample.*" },
		["Helpers"] = { "GLORY_YAML.*", "YAML_GLM.*", "ComponentTypes.*" },
		["Serialization"] = { "SceneObjectRefSerializer.*", "StructPropertySerializer.*", "AssetReference.*", "SceneObjectRef.*", "AssetRef.*", "LayerRef.*", "EnumPropertySerializer.*", "PropertyFlags.*", "PropertySerializer.*", "SerializedProperty.*", "Serializers.*", "AssetReferencePropertySerializer.*", "SerializedArrayProperty.*", "SerializedPropertyManager.*", "SerializedTypes.*", "ArrayPropertySerializer.*", "AssetReferencePropertyTemplate.*", "ShapePropertySerializer.*", "ScriptedComponentSerializer.*" },
		["Scripting"] = { "ScriptExtensions.*", "ScriptingExtender.*", "ScriptProperty.*" },
		["SceneManagement"] = { "UUIDRemapper.*", "SceneManager.*", "SceneObject.*", "GScene.*", "Components.*", "Entity.*", "EntityComponentObject.*", "SceneObject.*" },
		["EntitySystems"] = { "CharacterControllerSystem.*", "ModelRenderSystem.*", "ScriptedSystem.*", "MeshRenderSystem.*", "Systems.*", "TransformSystem.*", "CameraSystem.*", "LookAtSystem.*", "SpinSystem.*", "LightSystem.*", "MeshFilterSystem.*", "PhysicsSystem.*" },
	}

	includedirs
	{
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.jobs}",

		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",

		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",

		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
		"%{stb_image}/..",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_CORE_EXPORTS",
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
