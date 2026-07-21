ImGuiDir = "%{SubmodoleDirs.ImGui}"

local success, err = os.copyfile("imconfig.h", "../../submodules/ImGui/imconfig.h")
if not success then
    print(" Premake Copy Failed! Reason: " .. tostring(err))
end

project "ImGui"
	location "%{ImGuiDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
        "%{ImGuiDir}/imgui.h",
        "%{ImGuiDir}/imgui.cpp",
        "%{ImGuiDir}/imgui_draw.cpp",
        "%{ImGuiDir}/imgui_internal.h",
        "%{ImGuiDir}/imgui_tables.cpp",
        "%{ImGuiDir}/imgui_widgets.cpp",
        "%{ImGuiDir}/imstb_rectpack.h",
        "%{ImGuiDir}/imstb_textedit.h",
        "%{ImGuiDir}/imstb_truetype.h",
        "%{ImGuiDir}/imconfig.h",
        "imconfig.h",
        "premake5.lua",
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
