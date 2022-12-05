ImGuizmoDir = "%{SubmodoleDirs.ImGuizmo}"

project "ImGuizmo"
	location "%{ImGuizmoDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
        "%{ImGuizmoDir}/*.h",
        "%{ImGuizmoDir}/*.cpp",
	}

	sysincludedirs
    {
        "%{ImGuizmoDir}"
    }
        
    includedirs
    {
        "%{ImGuizmoDir}",
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
