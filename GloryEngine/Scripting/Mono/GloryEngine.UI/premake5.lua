project "GloryEngine.UI"
	kind "SharedLib"
	language "C#"
	namespace ("GloryEngine.UI")

	targetdir ("%{engineOutDir}/Mono/GloryEngine.UI")
	objdir ("%{outputDir}")

	dotnetframework "4.7.2"

	files
	{
		"*.cs",
		"Components/*.cs",
		"premake5.lua"
	}

	defines
	{
		"TRACE"
	}

	links
	{
		"GloryEngine.Core"
	}

	dependson
	{
		"GloryUIRenderer"
	}

	filter "system:windows"
		defines
		{
			"_LIB"
		}

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		optimize "Off"
		defines "DEBUG"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		defines "NDEBUG"
		symbols "Default"
