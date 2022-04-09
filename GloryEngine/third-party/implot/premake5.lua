ImPlotDir = "../%{SubmodoleDirs.implot}"

project "implot"
	location "%{ImPlotDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputdir}")
	objdir ("$(ProjectDir)/%{outputdir}")

	files
	{
        "%{ImPlotDir}/*.h",
        "%{ImPlotDir}/*.cpp",
	}

	sysincludedirs
    {
        "%{ImPlotDir}"
    }
        
    includedirs
    {
        "%{ImPlotDir}",
        "../../submodules/ImGui",
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
