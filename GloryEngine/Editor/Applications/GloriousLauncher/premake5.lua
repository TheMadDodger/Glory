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
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.FA}",

		"%{IncludeDir.glm}",
		"%{DepIncludesDir}",
		"%{IncludeDir.SDL_config}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.GloryECS}",

		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GLEW}",
		"%{DepDirs.SDL}",
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

	dependson
	{
		"Glorious",
	}

	postbuildcommands
	{
		("{COPY} ./Fonts %{buildDir}/Launcher/Fonts"),
		("{COPY} %{DepDirs.SDL}/*.dll %{buildDir}/Launcher"),
		("{COPY} %{LibDirs.GLEW}/*.dll %{buildDir}/Launcher"),
		("{COPY} imgui.ini %{buildDir}/Launcher"),
		("{COPY} %{IncludeDir.FA}/FA %{buildDir}/Launcher/Fonts/FA"),

		-- Copy editor to launcher
		("{COPY} %{editorBuildDir} %{buildDir}/Launcher/Editor/Any"),
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v142"
		files { 'windows.rc', '../Icon/windows.ico' }
		vpaths { ['Resources/*'] = { 'windows.rc', '../Icon/windows.ico' } }

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			--"%{vulkanDir}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin32/*.dll %{buildDir}/Launcher")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			--"%{vulkanDir}/Third-Party/Bin"
		}

		postbuildcommands
		{
			--("{COPY} %{vulkanDir}/Third-Party/Bin/*.dll %{buildDir}/Launcher")
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{buildDir}/Launcher"

		links "SDL2d"

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "SDL2"
