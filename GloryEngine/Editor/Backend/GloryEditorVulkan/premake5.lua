project "GloryEditorVulkan"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryVulkanGraphics/Editor/Backend")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_vulkan.h",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_vulkan.cpp",
		"premake5.lua"
	}

	vpaths
	{
		
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{vulkanDir}/include",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.vulkan}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.threads}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.FA}",

		"%{SubmodoleDirs.ImGui}/backends/",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GloryVulkanGraphics",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"yaml-cpp",
		"GloryEditor",
		"GloryVulkanGraphics",
		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",

		"ImGui",
		"implot",
		"ImGuiTestEngine",
		"vulkan-1",
	}

	defines
	{
		"GLORY_EDITORVULKAN_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkanDir}/Lib32",
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Lib",
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
