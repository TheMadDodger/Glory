project "GloryMain"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("../Build/%{cfg.buildcfg}/%{cfg.platform}")
	objdir ("%{cfg.buildcfg}/%{cfg.platform}")

	pchheader "stdafx.h"
	pchsource "stdafx.cpp"

	files
	{
		"**.h",
		"**.cpp"
	}

	vpaths
	{
		["Header Files"] = { "stdafx.h" },
		["Source Files"] = { "GloryMain.cpp", "stdafx.cpp" }
	}

	includedirs
	{
		"%{vulkan_sdk}/include",
		"%{vulkan_sdk}/third-party/include",

		"%{IncludeDir.assimp}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.SDL_image}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",

		"%{GloryIncludeDir.assimp}",
		"%{GloryIncludeDir.entityscenes}",
		"%{GloryIncludeDir.basicrenderer}",
		"%{GloryIncludeDir.clusteredrenderer}",
		"%{GloryIncludeDir.opengl}",
		"%{GloryIncludeDir.sdlimage}",
		"%{GloryIncludeDir.sdlwindow}",
		"%{GloryIncludeDir.vulkan}",
		"%{GloryIncludeDir.ImGui}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",

		"../EntityScenesEditorExtension",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.assimp}",
		"%{LibDirs.GLEW}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.SDL_image}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",
		"%{LibDirs.extensions}",
	}

	links
	{
		"vulkan-1",
		"glew32",
		"GloryCore",
		"GlorySDLWindow",
		"GloryVulkanGraphics",
		"GloryOpenGLGraphics",
		"GlorySDLImage",
		"GloryASSIMPModelLoader",
		"GloryEntityScenes",
		"GloryClusteredRenderer",
		"assimp",
		"SDL2",
		"SDL2_image",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"yaml-cpp",
		"GloryEditor",

		"EntityScenesEditorExtension",
	}

	postbuildcommands
	{
		("{COPY} %{LibDirs.SDL_image}/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}"),
		("{COPY} %{LibDirs.GLEW}/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}"),
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v142"

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkan_sdk}/Lib32",
			"%{vulkan_sdk}/Third-Party/Bin32"
		}

		postbuildcommands
		{
			("{COPY} %{vulkan_sdk}/Third-Party/Bin32/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkan_sdk}/Lib",
			"%{vulkan_sdk}/Third-Party/Bin"
		}

		postbuildcommands
		{
			("{COPY} %{vulkan_sdk}/Third-Party/Bin/*.dll ../Build/%{cfg.buildcfg}/%{cfg.platform}")
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

		links
		{
			"spirv-cross-cd",
			"spirv-cross-cored",
			"spirv-cross-cppd",
			"spirv-cross-glsld",
			"spirv-cross-hlsld",
			"spirv-cross-msld",
			"spirv-cross-reflectd",
			"spirv-cross-utild",
		}

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links
		{
			"spirv-cross-c",
			"spirv-cross-core",
			"spirv-cross-cpp",
			"spirv-cross-glsl",
			"spirv-cross-hlsl",
			"spirv-cross-msl",
			"spirv-cross-reflect",
			"spirv-cross-util",
		}
