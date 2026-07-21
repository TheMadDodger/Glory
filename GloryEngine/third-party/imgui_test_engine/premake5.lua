ImGuiTestEngineDir = "%{SubmodoleDirs.ImGuiTestEngine}"

project "ImGuiTestEngine"
	location "%{ImGuiDir}"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("$(ProjectDir)/lib/%{outputDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
        "%{ImGuiTestEngineDir}/*.h",
        "%{ImGuiTestEngineDir}/*.cpp",
		"premake5.lua"
	}

    includedirs
    {
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.implot}",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
    }

	filter "system:windows"
		systemversion "latest"
		staticruntime "off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
