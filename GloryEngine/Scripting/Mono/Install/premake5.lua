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
		"GloryEngine.Entities",
	}

	postbuildcommands
	{
		("{COPY} %{engineOutDir}/Mono/GloryEngine.Entities/GloryEngine.Entities.dll \"%{modulesOutDir}/GloryMonoScripting/Scripting/csharp/\""),
	}
