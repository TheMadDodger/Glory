project "GloryMonoScripting"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp",
		"Module.yaml"
	}

	vpaths
	{
		["Core"] = { "GloryMono.*", "GloryMonoScipting.*" },
		["Binders"] = { "DebugBinder.*", "MathBinder.*", "AssemblyBinding.*" },
		["Resource"] = { "MonoScriptLoader.*", "MonoScript.*" }
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{mono_install}/include/mono-2.0",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",
		--"libmono-static-sgen",
	}

	defines
	{
		"GLORY_EXPORTS"
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
		("{COPY} ./Assets %{moduleOutDir}/Assets"),
		("{COPY} ./Resources %{moduleOutDir}/Resources"),
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

		libdirs
		{
			"%{mono_installx86}/lib",
		}

		postbuildcommands
		{
			("{COPY} \"%{mono_installx86}/bin/mono-2.0-sgen.dll\" ../Build/%{cfg.buildcfg}/%{cfg.platform}"),
			("{COPY} \"%{mono_installx86}/lib/mono/4.5/*\" ../Build/%{cfg.buildcfg}/%{cfg.platform}/mono/4.5/"),
			("{COPY} \"%{mono_installx86}/lib/mono/4.5/*\" ./mono/4.5/"),
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{mono_install}/lib",
		}

		postbuildcommands
		{
			("{COPY} \"%{mono_install}/bin/mono-2.0-sgen.dll\" ../Build/%{cfg.buildcfg}/%{cfg.platform}"),
			("{COPY} \"%{mono_install}/lib/mono/4.5/*\" ../Build/%{cfg.buildcfg}/%{cfg.platform}/mono/4.5/"),
			("{COPY} \"%{mono_install}/lib/mono/4.5/*\" ./mono/4.5/"),
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
