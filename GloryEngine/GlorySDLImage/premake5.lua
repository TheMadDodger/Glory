project "GlorySDLImage"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Module"] = { "SDLImageLoaderModule.h", "SDLImageLoaderModule.cpp", "SDLTexture2D.h", "SDLTexture2D.cpp" }
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.SDL_image}",
		"%{GloryIncludeDir.core}"
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
