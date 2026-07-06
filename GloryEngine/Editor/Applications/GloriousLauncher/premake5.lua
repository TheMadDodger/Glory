project "GloriousLauncher"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{buildDir}/Launcher")
	objdir ("%{outputDir}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files
	{
		"**.h",
		"**.cpp",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_opengl3.h",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_opengl3.cpp",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_sdl2.h",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_sdl2.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{DepsIncludeDir}",
		"%{DepsIncludeDir}/SDL2",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.api}",

		"%{SubmodoleDirs.ImGui}/backends",

		"%{IncludeDir.Version}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.FA}",
		"%{IncludeDir.Utils}",

		"%{rapidjson}",

		"%{rootDir}/third-party/tinyfiledialogs"
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryAPI",
		"GloryUtilsVersion",
		"GloryUtils",

		"ImGui",
		"ImGuiTestEngine",
		"yaml-cpp",
		"tinyfiledialogs",
		"SDL2",
	}

	dependson
	{
		"Glorious",
	}

	postbuildcommands
	{
		("{COPY} ./Fonts %{buildDir}/Launcher/Fonts"),
		("{COPY} %{DepsBinDir}/SDL2?.dll %{buildDir}/Launcher"),
		("{COPY} %{DepsBinDir}/glew32*.dll %{buildDir}/Launcher"),
		("{COPY} imgui.ini %{buildDir}/Launcher"),
		("{COPY} %{IncludeDir.FA}/FA %{buildDir}/Launcher/Fonts/FA"),
		("{COPY} %{engineOutDir}/GloryAPI.dll %{buildDir}/Launcher"),
		("{COPY} %{DepsBinDir}/libcurl*.dll %{buildDir}/Launcher"),

		-- Copy editor to launcher
		("{COPY} %{editorBuildDir} %{buildDir}/Launcher/Editor/Any"),

	}

	postbuildcommands
	{
		("{COPY} %{DepsBinDir}/libcurl*.dll %{engineOutDir}"),
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"
		files { 'windows.rc', '../Icon/windows.ico' }
		vpaths { ['Resources/*'] = { 'windows.rc', '../Icon/windows.ico' } }

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{buildDir}/Launcher"

		links
		{
			"glew32d"
		}

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links
		{
			"glew32"
		}

	filter "files:**/imgui_impl_opengl3.cpp or files:**/imgui_impl_sdl2.cpp"
		flags { "NoPCH" }
	filter ""