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
		["Asset Management"] = { "BinaryStream.*", "AssetArchive.*", "AssetDatabase.*", "AssetLocation.*", "AssetManager.*", "ResourceMeta.*", "ResourceType.*" },
		["Resources"] = { "PrefabData.*", "MaterialData.*", "MaterialPropertyInfo.*", "TextureData.*", "MeshData.*", "ModelData.*", "ImageData.*", "FileData.*", "CubemapData.*", "FontData.*", "FontDataStructs.*", "PipelineData.*" },
		["Console"] = { "Logs.*", "CommandLine.*", "Commands.*", "Console.*", "Debug.*", "DebugConsoleInput.*", "IConsole.*", "WindowsDebugConsole.*" },
		["Core"] = { "ObjectManager.*", "TypeFlags.*", "GloryContext.*", "BuiltInModules.*", "Engine.*", "Object.*", "UUID.*", "Glory.*", "GameTime.*" },
		["Graphics"] = { "GraphicsDevice.*", "GraphicsEnums.*", "VertexDefinitions.*", "VertexHelpers.*", "RenderFrame.*" },
		["Modules/Base"] = { "Module.*", "ModuleMetaData.*", "IModuleLoopHandler.*", "ModuleSettings.*" },
		["Modules/Renderer/Camera"] = { "Camera.*", "CameraManager.*", "CameraRef.*" },
		["Modules/Renderer/Layers"] = { "Layer.*", "LayerManager.*", "LayerMask.*" },
		["Modules/Renderer/Data"] = { "LightData.*", "RenderData.*" },
		["Modules/ResourceLoading"] = {  },
		["Modules/ResourceLoading/Base"] = { "ImportSettings.*", "Resource.*", "ResourceLoaderModule.*" },
		["Modules/ResourceLoading/File"] = { "FileLoaderModule.*" },
		["Modules/ResourceLoading/Models"] = { "ModelLoaderModule.*" },
		["Modules/ResourceLoading/Textures"] = { "TextureDataLoaderModule.*" },
		["Modules/Window"] = { "Window.*", "WindowModule.*" },
		["Modules/Input"] = { "InputModule.*", "Input.*", "KeyEnums.*", "PlayerInput.*" },
		["Modules/Physics"] = { "ShapeManager.*", "CharacterManager.*", "PhysicsModule.*", "Shapes.*", "Physics.*", "ShapeProperty.*" },
		["Analysis"] = { "EngineProfiler.*", "ProfilerModule.*", "ProfilerSample.*", "ProfilerThreadSample.*" },
		["Helpers"] = { "GLORY_YAML.*", "YAML_GLM.*", "ComponentTypes.*" },
		["Serialization"] = { "SceneObjectRefSerializer.*", "StructPropertySerializer.*", "AssetReference.*", "SceneObjectRef.*", "AssetRef.*", "LayerRef.*", "EnumPropertySerializer.*", "PropertyFlags.*", "PropertySerializer.*", "SerializedProperty.*", "Serializers.*", "AssetReferencePropertySerializer.*", "SerializedArrayProperty.*", "SerializedPropertyManager.*", "SerializedTypes.*", "ArrayPropertySerializer.*", "AssetReferencePropertyTemplate.*", "ShapePropertySerializer.*", "ScriptedComponentSerializer.*" },
		["Scripting"] = { "ScriptExtensions.*", "ScriptingExtender.*", "ScriptProperty.*" },
		["SceneManagement"] = { "UUIDRemapper.*", "SceneManager.*", "SceneObject.*", "GScene.*", "Components.*", "Entity.*", "EntityComponentObject.*", "SceneObject.*" },
		["EntitySystems"] = { "CharacterControllerSystem.*", "ModelRenderSystem.*", "ScriptedSystem.*", "MeshRenderSystem.*", "Systems.*", "TransformSystem.*", "CameraSystem.*", "LookAtSystem.*", "SpinSystem.*", "LightSystem.*", "MeshFilterSystem.*" },
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
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	filter "system:windows"
		systemversion "latest"
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
