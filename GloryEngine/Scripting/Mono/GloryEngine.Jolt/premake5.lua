project "GloryEngine.Jolt"
	kind "SharedLib"
	language "C#"
	namespace ("GloryEngine")

	targetdir ("%{engineOutDir}/Mono/GloryEngine.Jolt")
	objdir ("%{outputDir}")

	dotnetframework "4.7.2"

	files
	{
		"*.cs",
		"Physics/*.cs",
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
		"GloryJoltPhysics"
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
