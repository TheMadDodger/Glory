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
		"%{DepsIncludeDir}",

		"%{GloryIncludeDir.core}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",

		"%{DepIncludesDir}",
		"%{IncludeDir.glm}",

		"%{IncludeDir.yaml_cpp}",
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
		"yaml-cpp",

		"GloryReflect",
		"GloryECS",
		"GloryUtilsVersion",
		"GloryUtils",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
		"GloryUtils",
	}

	defines
	{
		"GLORY_EXPORTS"
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
		("{COPY} %{DepsBinDir}/SDL2_image*.dll %{moduleOutDir}/Dependencies"),
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
			--("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			--"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links "SDL2d"
		links "SDL2_imaged"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "SDL2"
		links "SDL2_image"
