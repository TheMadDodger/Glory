project "GloryEngine.Editor"
	kind "SharedLib"
	language "C#"
	staticruntime "Off"
	namespace ("GloryEngine.Editor")

	targetdir ("%{modulesOutDir}/GloryMonoScripting/Editor/Scripting")
	objdir ("%{outputDir}")

	dotnetframework "4.7.1"

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
		"MonoEditorExtension"
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
