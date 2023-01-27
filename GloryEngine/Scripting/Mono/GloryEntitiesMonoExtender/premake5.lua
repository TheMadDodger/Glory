project "GloryEntitiesMonoExtender"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryEntityScenes/Scripting/csharp")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		["API"] = { "EntityCSAPI.*" },
		["Extenstion"] = { "EntitiesMonoExtender.*" },
		["Memory"] = { "MonoEntityObjectManager.*", "MonoEntitySceneManager.*" },
	}

	includedirs
	{
		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.entityscenes}",
		"%{GloryIncludeDir.mono}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}"
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"GloryEntityScenes",
		"GloryMonoScripting",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",

		"GloryECSStatic",
		"GloryReflectStatic"
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_UUID_DEFINED"
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
			"%{monox86Dir}/lib",
		}

		includedirs
		{
			"%{monox86Dir}/include/mono-2.0",
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{monoDir}/lib",
		}

		includedirs
		{
			"%{monoDir}/include/mono-2.0",
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
