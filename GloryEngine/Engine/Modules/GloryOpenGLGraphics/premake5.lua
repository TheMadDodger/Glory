project "GloryOpenGLGraphics"
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
		["Module"] = { "GloryOGL.*", "GLConverter.h", "GLConverter.cpp", "OpenGLGraphicsModule.h", "OpenGLGraphicsModule.cpp" },
		["Resources"] = { "GLBuffer.h", "GLBuffer.cpp", "GLMesh.h", "GLMesh.cpp", "GLShader.h", "GLShader.cpp", "GLTexture.h", "GLTexture.cpp", "OGLMaterial.h", "OGLMaterial.cpp", "OGLResourceManager.h", "OGLResourceManager.cpp", "OGLRenderTexture.h", "OGLRenderTexture.cpp" }
	}

	includedirs
	{
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",

		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{DepDirs.GLEW}/lib/%{cfg.buildcfg}",
		"%{DepDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
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

		("{COPY} %{DepDirs.GLEW}/bin/%{cfg.buildcfg}/*.dll %{moduleOutDir}/Dependencies"),
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

		links "glew32d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "glew32"
