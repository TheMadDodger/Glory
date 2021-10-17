project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineoutdir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Core"] = { "imconfig.h", "imgui.h", "imgui.h", "imgui.cpp", "imgui_demo.cpp", "imgui_draw.cpp", "imgui_internal.h", "imgui_tables.cpp", "imgui_widgets.cpp", "imstb_rectpack.h", "imstb_textedit.h", "imstb_truetype.h" }
	}

	includedirs
	{
		
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v142"

		defines
		{
			"_LIB"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter {"system:windows", "configurations:Release" }
		buildoptions "/MDd"

	filter {"system:windows", "configurations:Release" }
		buildoptions "/MD"