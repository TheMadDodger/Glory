project "GloryMonoScripting"
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
		"premake5.lua",
		"Assets/**.*",
		"Resources/**.*",
	}

	vpaths
	{
		["Core"] = { "GloryMono.*", "GloryMonoScipting.*" },
		["API"] = { "InputCSAPI.*", "CoreCSAPI.*", "MathCSAPI.*", "AssemblyBinding.*" },
		["Resource"] = { "MonoScriptLoader.*", "MonoScript.*" },
		["Mono"] = { "MonoAssetManager.*", "MonoLibManager.*", "MonoManager.*", "MonoObjectManager.*" }
	}

	includedirs
	{
		"%{vulkanDir}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",

		"%{IncludeDir.ECS}",
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
		"GloryCore",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",

		"GloryECSStatic",
		"GloryReflectStatic",
	}

	dependson
	{
		"Glorious"
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_UUID_DEFINED"
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
			"%{monox86Dir}/lib",
		}

		includedirs
		{
			"%{monox86Dir}/include/mono-2.0",
		}

		postbuildcommands
		{
			("{COPY} \"%{monox86Dir}/bin/mono-2.0-sgen.dll\" %{moduleOutDir}/Dependencies"),
			("{COPY} \"%{monox86Dir}/lib/mono/4.5/*\" %{moduleOutDir}/Dependencies/mono/4.5/"),
			--("{COPY} \"%{monox86Dir}/lib/mono/4.5/*\" ./mono/4.5/"),
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

		postbuildcommands
		{
			("{COPY} \"%{monoDir}/bin/mono-2.0-sgen.dll\" %{moduleOutDir}/Dependencies"),
			("{COPY} \"%{monoDir}/lib/mono/4.5/*\" %{moduleOutDir}/Dependencies/mono/4.5/"),
			--("{COPY} \"%{monoDir}/lib/mono/4.5/*\" ./mono/4.5/"),
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
