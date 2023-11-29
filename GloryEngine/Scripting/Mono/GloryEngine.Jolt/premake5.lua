project "GloryEngine.Jolt"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("GloryEngine")

	targetdir ("%{engineOutDir}/Mono/GloryEngine.Jolt")
	objdir ("%{outputDir}")

	dotnetframework "4.7.1"

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
