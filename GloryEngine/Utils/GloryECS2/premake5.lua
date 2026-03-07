project "GloryECS2"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"*.h",
		"*.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		--"%{GloryIncludeDir.enginecore}",

		"%{IncludeDir.Utils}",
	}

	links
	{
		"GloryUtils"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

		defines
		{
			--"_LIB"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
