project "GloryASSIMPModelLoader"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp",
		"Module.yaml"
	}

	vpaths
	{
		["Module"] = { "ASSIMPModule.h", "ASSIMPModule.cpp" }
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.yaml_cpp}",
		"%{vulkan_sdk}/third-party/include",
		
		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.assimp}",
		"%{LibDirs.yaml_cpp}",
		
		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"assimp",
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
