project "GloryAPI"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

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
		"%{DepsIncludeDir}",
		"%{IncludeDir.Version}",

		"%{rapidjson}"
	}

	libdirs
	{
		"%{DepsLibDir}",
	}

	defines
	{
		"GLORY_API_EXPORTS",
		"GLORY_ENABLE_API",
		"GLORY_NO_DEBUG_LINES"
	}

	postbuildcommands
	{
		("{COPY} %{DepsBinDir}/libcurl*.dll %{engineOutDir}"),
	}

	links
	{
		"GloryReflect",
		"GloryUtilsVersion"
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v143"

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
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links "libcurl-d_imp"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "libcurl_imp"
