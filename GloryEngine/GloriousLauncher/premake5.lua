project "GloriousLauncher"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.GLEW}",
		
		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.yaml_cpp}",
		"%{LibDirs.GLEW}",
		
		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"ImGui",
		"ImFileDialog",
		"yaml-cpp",
		"glew32",
		
		"GloryReflectStatic",
	}

	postbuildcommands
	{
		("{COPY} ./Fonts ../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}/Fonts"),
		("{COPY} %{LibDirs.GLEW}/*.dll ../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}"),
		("{COPY} imgui.ini ../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}"),
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v142"

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkan_sdk}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			("{COPY} %{vulkan_sdk}/Third-Party/Bin32/*.dll ../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkan_sdk}/Third-Party/Bin"
		}

		postbuildcommands
		{
			("{COPY} %{vulkan_sdk}/Third-Party/Bin/*.dll ../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "../Build/Launcher/%{cfg.buildcfg}/%{cfg.platform}"

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
