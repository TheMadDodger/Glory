project "GloryThreads"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp",
		"premake5.lua"
	}

	vpaths
	{
		["Threading"] = { "Thread.*", "ThreadManager.*", "ThreadedVar.*" },
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",

		"%{IncludeDir.Reflect}",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_CORE_EXPORTS",
	}

	filter "system:windows"
		systemversion "latest"
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

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
