project "MonoEditorExtension"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryMonoScripting/Editor/Extension")
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
		
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.mono}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.mono}",
		
		"%{mono_install}/include/mono-2.0",
	}
	
	libdirs
	{
		"%{LibDirs.ImGui}",
		"%{LibDirs.LibDirs}",
		"%{LibDirs.implot}",
		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GloryMonoScripting",
		"%{LibDirs.assimp}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",
	}
	
	links
	{
		"GloryCore",
		"GloryEditor",
		"GloryMonoScripting",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",
		"mono-2.0-sgen",
		"MonoPosixHelper",
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

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
