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
		["Asset Management"] = { "AssetDatabase.*", "AssetGroup.*", "AssetLocation.*", "AssetManager.*", "ResourceMeta.*", "ResourceType.*", "ShaderManager.*", "AssetCallbacks.*" },
		["Console"] = { "Logs.*", "CommandLine.*", "Commands.*", "Console.*", "Debug.*", "DebugConsoleInput.*", "IConsole.*", "WindowsDebugConsole.*" },
		["Core"] = { "ObjectManager.*", "TypeFlags.*", "GloryContext.*", "BuiltInModules.*", "CoreExceptions.*", "Engine.*", "Game.*", "GameSettings.*", "GameState.*", "GloryCore.*", "GraphicsThread.*", "Object.*", "UUID.*", "Glory.*" },
		["Layers"] = { "Layer.*", "LayerManager.*", "LayerMask.*" },
		["Modules/Base"] = { "Module.*", "ModuleMetaData.*", "IModuleLoopHandler.*", "ModuleSettings.*" },
		["Modules/Time"] = { "TimerModule.*", "GameTime.*" },
		["Modules/ResourceLoading"] = {  },
		["Modules/ResourceLoading/Base"] = { "ImportSettings.*", "Resource.*", "ResourceLoaderModule.*" },
		["Modules/ResourceLoading/File"] = { "FileData.*", "FileLoaderModule.*" },
		["Modules/ResourceLoading/Material"] = { "MaterialData.*", "MaterialPropertyData.*", "MaterialInstanceData.*", "MaterialInstanceLoaderModule.*", "MaterialLoaderModule.*", "MaterialPropertyInfo.*" },
		["Modules/ResourceLoading/Models"] = { "MeshData.*", "ModelData.*", "ModelLoaderModule.*" },
		["Modules/ResourceLoading/Shaders"] = { "ShaderCrossCompiler.*", "ShaderData.*", "ShaderLoaderModule.*", "ShaderSourceData.*", "ShaderSourceLoaderModule.*" },
		["Modules/ResourceLoading/Images"] = { "ImageData.*", "ImageLoaderModule.*" },
		["Modules/ResourceLoading/Textures"] = { "TextureData.*", "TextureDataLoaderModule.*" },
		["Modules/Scripting"] = { "Script.*", "ScriptingModule.*", "ScriptLoaderModule.*", "ScriptBinding.*", "ScriptingBinder.*", "IScriptExtender.*" },
		["Analysis"] = { "EngineProfiler.*", "ProfilerModule.*", "ProfilerSample.*", "ProfilerThreadSample.*" },
		["Helpers"] = { "GLORY_YAML.*", "YAML_GLM.*" },
		["Serialization"] = { "SceneObjectRefSerializer.*", "StructPropertySerializer.*", "AssetReference.*", "SceneObjectRef.*", "AssetRef.*", "LayerRef.*", "EnumPropertySerializer.*", "PropertyFlags.*", "PropertySerializer.*", "SerializedProperty.*", "Serializer.*", "AssetReferencePropertySerializer.*", "SerializedArrayProperty.*", "SerializedPropertyManager.*", "SerializedTypes.*", "ArrayPropertySerializer.*", "AssetReferencePropertyTemplate.*", "ShapePropertySerializer.*" },
		["Scripting"] = { "ScriptExtensions.*", "ScriptingExtender.*", "ScriptProperty.*" },
		["Utils"] = { "NodeRef.*" },
		["Geometry"] = { "VertexDefinitions.*", "VertexHelpers.*" },
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
