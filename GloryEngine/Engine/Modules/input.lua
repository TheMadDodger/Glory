project "GloryModulesInput"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"InputModule.*",
		"Input.*",
		"KeyEnums.*",
		"PlayerInput.*",
		"input.lua",
		"premake5.lua",
	}

	vpaths
	{
		--["Modules/Input"] = { "InputModule.*", "Input.*", "KeyEnums.*", "PlayerInput.*" },
	}

	includedirs
	{
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",

		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",

		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_CORE_EXPORTS",
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

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
