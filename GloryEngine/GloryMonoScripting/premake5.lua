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

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
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

		"GloryECSStatic",
		"GloryReflectStatic",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLORY_UUID_DEFINED"
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
		
		includedirs
		{
			"%{mono_installx86}/include/mono-2.0",
		}
		
		postbuildcommands
		{
			("{COPY} \"%{mono_installx86}/bin/mono-2.0-sgen.dll\" %{moduleOutDir}/Dependencies"),
			("{COPY} \"%{mono_installx86}/lib/mono/4.5/*\" %{moduleOutDir}/Dependencies/mono/4.5/"),
			--("{COPY} \"%{mono_installx86}/lib/mono/4.5/*\" ./mono/4.5/"),
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{mono_install}/lib",
		}
		
		includedirs
		{
			"%{mono_install}/include/mono-2.0",
		}

		postbuildcommands
		{
			("{COPY} \"%{mono_install}/bin/mono-2.0-sgen.dll\" %{moduleOutDir}/Dependencies"),
			("{COPY} \"%{mono_install}/lib/mono/4.5/*\" %{moduleOutDir}/Dependencies/mono/4.5/"),
			--("{COPY} \"%{mono_install}/lib/mono/4.5/*\" ./mono/4.5/"),
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
