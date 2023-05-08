project "tinyfiledialogs"
	kind "StaticLib"
	language "C++"

	targetdir ("%{engineOutDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
        "tinyfiledialogs.h",
        "tinyfiledialogs.c",
	}

	sysincludedirs
    {
        
    }

    includedirs
    {
        
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
