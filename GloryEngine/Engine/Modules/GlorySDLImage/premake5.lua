project "GlorySDLImage"
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
		["Module"] = { "GlorySDLImage.*", "SDLImageLoaderModule.h", "SDLImageLoaderModule.cpp", "SDLTexture2D.h", "SDLTexture2D.cpp" }
	}

	includedirs
	{
		"%{vulkanDir}/third-party/include",
		"%{IncludeDir.SDL_image}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",

		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{vulkanDir}/third-party/lib",
		"%{LibDirs.glory}",
		"%{LibDirs.SDL_image}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"SDL2",
		"SDL2_image",
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
		("{COPY} %{LibDirs.SDL_image}/*.dll %{moduleOutDir}/Dependencies"),
		--("{COPY} ./Assets %{moduleOutDir}/Assets"),
		--("{COPY} ./Resources %{moduleOutDir}/Resources"),
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
			--("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
