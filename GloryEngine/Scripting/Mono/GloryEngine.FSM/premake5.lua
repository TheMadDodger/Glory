project "GloryEngine.FSM"
	kind "SharedLib"
	language "C#"
	namespace ("GloryEngine.FSM")

	targetdir ("%{engineOutDir}/Mono/GloryEngine.FSM")
	objdir ("%{outputDir}")

	dotnetframework "4.7.2"

	files
	{
		"*.cs",
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
		"GloryFSM"
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
