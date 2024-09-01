project "GlorySteamAudio"
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
	}

	includedirs
	{
		"%{DepsIncludeDir}",
		"%{BaseIncludeDir.audio}",
		"%{BaseIncludeDir.audio3d}",

		"%{IncludeDir.glm}",
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",

		"%{IncludeDir.yaml_cpp}",

		"%{DepIncludesDir}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",

		"%{LibDirs.yaml_cpp}",

		"{moduleBaseOutDir}",
	}

	links
	{
		"GloryCore",
		"GloryAudioModule",
		"Glory3DAudioModule",
		"GloryReflect",
		"GloryECS",

		"phonon",

		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",

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
		("{COPY} %{DepsBinDir}/GPUUtilities*.dll %{moduleOutDir}/Dependencies"),
		("{COPY} %{DepsBinDir}/phonon*.dll %{moduleOutDir}/Dependencies"),
		("{COPY} %{DepsBinDir}/TrueAudioNext*.dll %{moduleOutDir}/Dependencies"),
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
