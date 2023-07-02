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
		"premake5.lua",
		"Assets/**.*",
		"Resources/**.*",
	}

	vpaths
	{
		["Core"] = { "SDLWindow.h", "SDLWindow.cpp", "SDLWindowExceptions.h" },
		["Module"] = { "GlorySDLWindow.*", "SDLWindowModule.h", "SDLWindowModule.cpp" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.Reflect}",

		"%{IncludeDir.yaml_cpp}",

		"%{DepIncludesDir}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",

		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"GloryReflect",

		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",
	}

	defines
	{
		"GLORY_EXPORTS"
	}

	postbuildcommands
	{
		("{COPY} %{DepsBinDir}/SDL2d?.dll %{moduleOutDir}/Dependencies"),
		("{COPY} %{DepsBinDir}/SDL2?.dll %{moduleOutDir}/Dependencies"),
		("{COPY} ./Module.yaml %{moduleOutDir}"),
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

		libdirs
		{
			--"%{vulkanDir}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll %{moduleOutDir}/Dependencies")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			--"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll %{moduleOutDir}/Dependencies")
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links "SDL2d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "SDL2"
