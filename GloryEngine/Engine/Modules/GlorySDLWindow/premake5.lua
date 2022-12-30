project "GlorySDLWindow"
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
		"premake5.lua"
	}

	vpaths
	{
		["Core"] = { "SDLWindow.h", "SDLWindow.cpp", "SDLWindowExceptions.h" },
		["Module"] = { "GlorySDLWindow.*", "SDLWindowModule.h", "SDLWindowModule.cpp" }
	}

	includedirs
	{
		"%{vulkanDir}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",

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
		"SDL2",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
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

		libdirs
		{
			"%{vulkanDir}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll %{moduleOutDir}/Dependencies")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll %{moduleOutDir}/Dependencies")
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
