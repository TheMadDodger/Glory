ImFileDialogDir = "../%{SubmodoleDirs.ImFileDialog}"

project "ImFileDialog"
	location "%{ImFileDialogDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputdir}")
	objdir ("$(ProjectDir)/%{outputdir}")

	files
	{
        "%{ImFileDialogDir}/ImFileDialog.h",
        "%{ImFileDialogDir}/ImFileDialog.cpp",
	}

	sysincludedirs
    {
        "%{ImFileDialogDir}"
    }
        
    includedirs
    {
        "%{ImFileDialogDir}",
        "%{stb_image}",
        "../../submodules",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
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
