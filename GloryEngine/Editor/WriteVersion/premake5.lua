project "WriteVersion"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{outputDir}")
	objdir ("%{outputDir}")

	files
	{
		"../GloryEditor/Configure.h",
		"main.cpp",
		"premake5.lua"
	}

	dependson
	{
		"GloryEditor",
	}

	postbuildcommands
	{
		("\"$(ProjectDir)%{outputDir}/WriteVersion.exe\" \"%{engineOutDir}\"")
	}
