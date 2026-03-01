project "GloryEngineCore"
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

	includedirs
	{
		--"%{GloryIncludeDir.threads}",
		--"%{GloryIncludeDir.jobs}",
		--"%{DepsIncludeDir}",
		--"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		--"%{IncludeDir.Reflect}",
		--"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		--"%{IncludeDir.ECS}",
		--"%{stb_image}/..",
	}

	defines
	{
		--"GLORY_EXPORTS",
		--"GLORY_CORE_EXPORTS",
		--"GLM_FORCE_RADIANS",
		--"GLM_FORCE_DEPTH_ZERO_TO_ONE"
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
