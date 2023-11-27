project "CSAPIInstall"
	kind "StaticLib"
	language "C++"
	staticruntime "Off"

	targetdir ("%{outputDir}")
	objdir ("%{outputDir}")

	files
	{
		"premake5.lua"
	}

	dependson
	{
		"GloryEngine.Core",
		"GloryEngine.Jolt",
	}

	postbuildcommands
	{
		("{COPY} %{engineOutDir}/Mono/GloryEngine.Jolt/GloryEngine.Jolt.dll \"%{modulesOutDir}/GloryJoltPhysics/Scripting/csharp/\""),
	}
