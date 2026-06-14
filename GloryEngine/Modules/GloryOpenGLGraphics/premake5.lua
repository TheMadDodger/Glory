project "GloryOpenGLGraphics"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"**.natvis",
		"Module.yaml",
		"premake5.lua",
		"Assets/**.*",
		"Resources/**.*",
	}

	vpaths
	{
		["Module"] = { "GloryOGL.*", "GLConverter.*", "OpenGLGraphicsModule.*", "OpenGLDevice.*" },
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",

		"yaml-cpp",

		"GloryReflect",
		"GloryECS",
		"GloryUtilsVersion",
		"GloryUtils",
	}

	defines
	{
		"GLORY_OGL_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
		("{COPY} %{DepsBinDir}/glew32*.dll %{moduleOutDir}/Dependencies"),
		("{COPY} ./Assets %{moduleOutDir}/Assets"),
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links "glew32d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "glew32"
