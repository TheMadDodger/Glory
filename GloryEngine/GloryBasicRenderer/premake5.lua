project "GloryBasicRenderer"
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
		["Header Files"] = { "framework.h", "pch.h" },
		["Source Files"] = { "GloryBasicRenderer.cpp", "pch.cpp" }
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{vulkan_sdk}/third-party/include"
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