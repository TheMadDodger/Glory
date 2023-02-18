project "GloryEditorOGL"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{modulesOutDir}/GloryOpenGLGraphics/Editor/Backend")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_opengl3.h",
		"%{SubmodoleDirs.ImGui}/backends/imgui_impl_opengl3.cpp",
		"premake5.lua"
	}

	vpaths
	{

	}

	includedirs
	{
		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.spirv_cross}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.opengl}",
		"%{GloryIncludeDir.editor}",

		"%{IncludeDir.FA}",
		"%{IncludeDir.Reflect}",

		"%{SubmodoleDirs.ImGui}/backends/",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{DepDirs.GLEW}/lib/%{cfg.buildcfg}",
		"%{LibDirs.glory}/Modules/GloryOpenGLGraphics",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
		"%{DepDirs.shaderc}",
		"%{DepDirs.spirv_cross}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"yaml-cpp",
		"GloryEditor",
		"GloryOpenGLGraphics",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"ImFileDialog",

		"GloryReflectStatic",
	}

	defines
	{
		"GLORY_EXPORTS"
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

		links "glew32d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "glew32"
