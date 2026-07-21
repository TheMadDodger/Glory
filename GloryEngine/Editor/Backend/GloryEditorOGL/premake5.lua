project "GloryEditorOGL"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
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
		"%{DepsIncludeDir}",

		"%{GloryIncludeDir.enginecore}",
		"%{GloryIncludeDir.engine}",
		"%{GloryIncludeDir.opengl}",
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
		"%{LibDirs.glory}/Modules/GloryOpenGLGraphics",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuiTestEngine}",
		"%{LibDirs.yaml_cpp}",
	}

	links
	{
		"GloryEngineCore",
		"GloryEngine",
		"GloryEditor",
		"GloryOpenGLGraphics",
		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",

		"yaml-cpp",
		"ImGui",
		"implot",
		"ImGuiTestEngine",
	}

	defines
	{
		"GLORY_EDITOROGL_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	filter "system:windows"
		systemversion "latest"
		toolset "v143"

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

		links
		{
			"glew32d",
			"spirv-cross-cd",
			"spirv-cross-cored",
			"spirv-cross-glsld",
		}

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links
		{
			"glew32",
			"spirv-cross-c",
			"spirv-cross-core",
			"spirv-cross-glsl",
		}
