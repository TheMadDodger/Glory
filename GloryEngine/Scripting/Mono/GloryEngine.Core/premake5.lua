project "GloryEngine.Core"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("GloryEngine")

	targetdir ("%{modulesOutDir}/GloryMonoScripting/Scripting/csharp")
	objdir ("%{outputDir}")
	
	dotnetframework "4.7.1"

	files
	{
		"**.cs",
		"premake5.lua"
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
