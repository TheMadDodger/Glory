project "GloryBasicRenderer"
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
		["Header Files"] = { "framework.h", "pch.h" },
		["Source Files"] = { "GloryBasicRenderer.cpp", "pch.cpp" }
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{vulkan_sdk}/third-party/include",
		
		"%{IncludeDir.Reflect}",
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
