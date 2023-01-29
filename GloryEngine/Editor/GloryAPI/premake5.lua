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
		"%{GloryIncludeDir.core}",

		"%{DepIncludesDirRoot}",
		"%{IncludeDir.rapidjson}"
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.GloryECS}",

		"%{DepDirs.curl}",
		"%{LibDirs.yaml_cpp}",
	}

	defines
	{
		"GLORY_API_EXPORTS",
	}

	postbuildcommands
	{
		("{COPY} %{DepDirs.curl}/*.dll %{engineOutDir}"),
	}

	links
	{
		"GloryCore",
		"yaml-cpp",
		"GloryReflectStatic",
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
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links "libcurl-d_imp"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "libcurl_imp"
