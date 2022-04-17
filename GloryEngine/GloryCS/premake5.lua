project "GloryCS"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("Glory")

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	framework "netstandard2.1"

	files
	{
		"*.cs",
	}

	vpaths
	{
	}

	includedirs
	{
	}

	defines
	{
		"TRACE"
	}

	filter "system:windows"
		--dotnetframework "4.7.2"

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
		defines "DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
