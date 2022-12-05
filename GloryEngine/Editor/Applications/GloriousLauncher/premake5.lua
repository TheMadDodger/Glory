project "GloriousLauncher"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{buildDir}/Launcher")
	objdir ("%{outputDir}")

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
		"%{vulkanDir}/third-party/include",
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
		("{COPY} ./Fonts %{buildDir}/Launcher/Fonts"),
		("{COPY} %{LibDirs.GLEW}/*.dll %{buildDir}/Launcher"),
		("{COPY} imgui.ini %{buildDir}/Launcher"),
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
			"%{vulkanDir}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll %{buildDir}/Launcher")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll %{buildDir}/Launcher")
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{buildDir}/Launcher"

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
