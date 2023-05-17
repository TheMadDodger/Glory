project "WriteVersion"
	kind "ConsoleApp"
	language "C++"
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
		("\"$(ProjectDir)Debug\\x64\\WriteVersion.exe\""),
		("{COPY} version.txt %{engineOutDir}"),
	}
