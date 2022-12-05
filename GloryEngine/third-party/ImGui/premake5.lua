ImGuiDir = "%{SubmodoleDirs.ImGui}"

project "ImGui"
	location "%{ImGuiDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
        "%{ImGuiDir}/*.h",
        "%{ImGuiDir}/*.cpp",
	}

	sysincludedirs
    {
        "%{ImGuiDir}"
    }
        
    includedirs
    {
        "%{ImGuiDir}",
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
