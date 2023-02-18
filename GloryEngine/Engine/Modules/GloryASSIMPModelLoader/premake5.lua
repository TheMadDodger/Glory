project "GloryASSIMPModelLoader"
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
		["Module"] = { "GloryASSIMPModelLoader.*", "GloryClusteredRenderer.*", "ASSIMPModule.h", "ASSIMPModule.cpp" }
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.glm}",

		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",
		"%{LibDirs.GloryECS}",

		"%{DepDirs.assimp}/lib/%{cfg.buildcfg}",
	}

	links
	{
		"GloryCore",
		"yaml-cpp",
		"GloryReflectStatic",
	}

	defines
	{
		"GLORY_EXPORTS"
	}

	prebuildcommands
	{
		("{DELETE} %{moduleOutDir}/Module.yaml"),
	}

	postbuildcommands
	{
		("{COPY} Module.yaml %{moduleOutDir}"),
		("{COPY} %{DepDirs.assimp}/bin/%{cfg.buildcfg}/*.dll %{moduleOutDir}/Dependencies"),
		("{COPY} ./Assets %{moduleOutDir}/Assets"),
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

		links "assimp-vc143-mtd"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "assimp-vc143-mt"
