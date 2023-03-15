project "GloryJoltPhysics"
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

		"%{rootDir}/submodules/JoltPhysics/Jolt/**.*",
	}

	vpaths
	{
		
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

	sysincludedirs
	{
		"%{rootDir}/submodules/JoltPhysics",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.GloryECS}",

		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"GloryReflectStatic",

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

		links "SDL2d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "SDL2"
