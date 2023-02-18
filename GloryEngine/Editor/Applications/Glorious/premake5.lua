project "Glorious"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{engineOutDir}")
	objdir ("%{outputDir}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files
	{
		"*.h",
		"*.cpp",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",
		"%{IncludeDir.ImFileDialog}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.spirv_cross}",
		"%{IncludeDir.yaml_cpp}",
		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.editor}",
		"%{engineDir}/GloryEngineBuilder",
		"%{IncludeDir.FA}",

		"%{IncludeDir.Reflect}",
	}

	libdirs
	{
		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.ImFileDialog}",
		"%{LibDirs.shaderc}",
		"%{LibDirs.spirv_cross}",
		"%{LibDirs.yaml_cpp}",

		"%{LibDirs.GloryECS}",
	}

	links
	{
		"GloryCore",
		"GloryEngineBuilder",
		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"ImGui",
		"ImGuizmo",
		"implot",
		"ImFileDialog",
		"yaml-cpp",
		"GloryEditor",

		"GloryReflectStatic",
	}

	dependson
	{
		"GloryASSIMPModelLoader",
		"GloryClusteredRenderer",
		"GloryEntityScenes",
		"GloryOpenGLGraphics",
		--"GloryVulkanGraphics",
		"GlorySDLImage",
		"GlorySDLWindow",
		"GlorySDLInput",
		"GloryEditorSDL",
		"GloryEditorOGL",
		--"GloryEditorVulkan",
		"GloryMonoScripting",
		"EntityScenesEditorExtension",
		"MonoEditorExtension",
		"GloryEntitiesMonoExtender",
		"GloryEngine.Core",
		"GloryEngine.Entities",
	}

	postbuildcommands
	{
		("{COPY} %{modulesOutDir} %{editorBuildDir}/Modules"),
		("{COPY} %{engineOutDir}/GloryEditor.dll %{editorBuildDir}"),
		("{COPY} ./EditorAssets %{editorBuildDir}/EditorAssets"),
		("{COPY} ./EditorAssets %{engineOutDir}/EditorAssets"),
		("{COPY} EditorSettings.yaml %{editorBuildDir}"),
		("{COPY} EditorSettings.yaml %{engineOutDir}"),
		("{COPY} imgui.ini %{editorBuildDir}"),
		("{COPY} imgui.ini %{engineOutDir}"),
		("{COPY} %{premakeDir} %{editorBuildDir}/premake"),
		("{COPY} %{premakeDir} %{engineOutDir}/premake"),
		("{COPY} %{engineOutDir}/Glorious.exe %{editorBuildDir}"),
		("{COPY} ./Templates %{editorBuildDir}/Templates"),
		("{COPY} ./Templates %{engineOutDir}/Templates"),
		("{COPY} %{IncludeDir.FA}/FA %{editorBuildDir}/EditorAssets/Fonts/FA"),
		("{COPY} %{IncludeDir.FA}/FA %{engineOutDir}/EditorAssets/Fonts/FA"),
	}

	filter "system:windows"
		systemversion "10.0.19041.0"
		toolset "v143"
		files { 'windows.rc', '../Icon/windows.ico' }
		vpaths { ['Resources/*'] = { 'windows.rc', '../Icon/windows.ico' } }

		defines
		{
			"_CONSOLE"
		}

	filter "platforms:Win32"
		architecture "x86"
		defines "WIN32"

		libdirs
		{
			"%{vulkanDir}/Lib32",
			"%{vulkanDir}/Third-Party/Bin32"
		}

	filter "platforms:x64"
		architecture "x64"

		libdirs
		{
			"%{vulkanDir}/Lib",
			"%{vulkanDir}/Third-Party/Bin"
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"
		debugdir "%{engineOutDir}"
		debugargs { "-projectPath=\"%{demosDir}/Sponza/Sponza.gproj\"" }

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
