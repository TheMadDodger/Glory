project "GloryMain"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("Build/%{cfg.buildcfg}/%{cfg.platform}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	pchheader "stdafx.h"
	pchsource "stdafx.cpp"

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Header Files"] = { "stdafx.h" }
		["Source Files"] = { "GloryMain.cpp", "stdafx.cpp" }
	}

	includedirs
	{
		"%(vulkan_sdk)\third-party\include",
		"$(SolutionDir)bin\assimp\include"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v142"

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:x86"
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

