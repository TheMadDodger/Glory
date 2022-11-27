project "GloryEngine.Entities"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("GloryEngine.Entities")

	targetdir ("../%{engineoutdir}/Mono/GloryEngine.Entities")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")
	
	dotnetframework "4.7.1"
	
	files
	{
		"**.cs",
	}

	vpaths
	{
	}

	includedirs
	{
	}

	links
	{
		"GloryEngine.Core",
	}

	defines
	{
		"TRACE"
	}
	
	postbuildcommands
	{
		("{COPY} GloryEngine.Entities.dll \"../../../../../%{modulesOutDir}/GloryEntityScenes/Scripting/csharp/\""),
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
