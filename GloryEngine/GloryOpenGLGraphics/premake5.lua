project "GloryOpenGLGraphics"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Module"] = { "GLConverter.h", "GLConverter.cpp", "OpenGLGraphicsModule.h", "OpenGLGraphicsModule.cpp" }
		["Resources"] = { "GLBuffer.h", "GLBuffer.cpp", "GLMesh.h", "GLMesh.cpp", "GLShader.h", "GLShader.cpp", "GLTexture.h", "GLTexture.cpp", "OGLMaterial.h", "OGLMaterial.cpp", "OGLResourceManager.h", "OGLResourceManager.cpp" }
	}

	includedirs
	{
		"%(vulkan_sdk)\third-party\include",
		"$(SolutionDir)bin\assimp\include"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v142"

		defines
		{
			"_LIB"
		}

	filter "platforms:x86"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

