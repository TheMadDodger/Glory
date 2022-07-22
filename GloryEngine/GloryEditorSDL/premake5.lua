project "GloryEditorSDL"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GlorySDLWindow/Editor/Backend")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		
	}

	includedirs
	{
		"%{vulkan_sdk}/third-party/include",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.sdlwindow}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GlorySDLWindow",
		"%{LibDirs.ImGui}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryCore",
		"SDL2",
		"yaml-cpp",
		"ImGui",
		"GloryEditor",
		"GlorySDLWindow",
	}

	defines
	{
		"GLORY_EXPORTS"
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
			"%{vulkan_sdk}/Third-Party/Bin32"
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkan_sdk}/Third-Party/Bin"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
