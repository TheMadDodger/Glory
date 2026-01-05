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
		"%{DepsIncludeDir}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGui}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.opengl}",
		"%{GloryIncludeDir.editor}",
		"%{GloryIncludeDir.threads}",

		"%{IncludeDir.FA}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",

		"%{SubmodoleDirs.ImGui}/backends/",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.glory}/Modules/GloryOpenGLGraphics",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
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

		"GloryECS",
		"GloryReflect",
		"GloryUtilsVersion",
		"GloryUtils",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
		"GloryUtils",
	}

	defines
	{
		"GLORY_EXPORTS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
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

		links
		{
			"glew32d",
			"spirv-cross-cd",
			"spirv-cross-cored",
			--"spirv-cross-cppd",
			"spirv-cross-glsld",
			--"spirv-cross-hlsld",
			--"spirv-cross-msld",
			--"spirv-cross-reflectd",
			--"spirv-cross-utild",
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
			--"spirv-cross-cpp",
			"spirv-cross-glsl",
			--"spirv-cross-hlsl",
			--"spirv-cross-msl",
			--"spirv-cross-reflect",
			--"spirv-cross-util",
		}
