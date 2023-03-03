project "GloryEngine.Entities"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("GloryEngine.Entities")

	targetdir ("%{engineOutDir}/Mono/GloryEngine.Entities")
	objdir ("%{outputDir}")

	dotnetframework "4.7.1"

	files
	{
		"*.cs",
		"Components/*.cs",
		"premake5.lua"
	}

	links
	{
		"GloryEngine.Core",
	}

	dependson
	{
		"GloryMonoScripting",
		"GloryEntitiesMonoExtender",
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
