project "GloryEngine.Core"
	kind "SharedLib"
	language "C#"
	namespace ("GloryEngine")

	targetdir ("%{modulesOutDir}/GloryMonoScripting/Scripting/csharp")
	objdir ("%{outputDir}")

	dotnetframework "4.7.2"

	files
	{
		"*.cs",
		"SceneManagement/*.cs",
		"AssetManagement/*.cs",
		"Layers/*.cs",
		"Math/*.cs",
		"Input/*.cs",
		"Entities/**.cs",
		"premake5.lua"
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
