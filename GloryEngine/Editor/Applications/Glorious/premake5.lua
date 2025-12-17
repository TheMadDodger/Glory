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
		"EditorAssets/**.*",
		"premake5.lua"
	}

	vpaths
	{
	}

	includedirs
	{
		"%{DepsIncludeDir}",

		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.implot}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.threads}",
		"%{GloryIncludeDir.editor}",
		"%{engineDir}/GloryEngineBuilder",

		"%{IncludeDir.FA}",

		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",
		"%{IncludeDir.ECS}",

		"%{rapidjson}",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",
		"%{LibDirs.ImGui}",
		"%{LibDirs.ImGuizmo}",
		"%{LibDirs.implot}",
		"%{LibDirs.yaml_cpp}",
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
		"yaml-cpp",
		"GloryEditor",
		"GloryReflect",

		"GloryECS",
		"GloryUtils",
		"GloryUtilsVersion",

		--todo: When asset management is contained in its own lib these links are no more needed
		"GloryJobs",
		"GloryThreads",
	}

	dependson
	{
		"GloryASSIMPModelLoader",
		"GloryOpenGLGraphics",
		"GloryVulkanGraphics",
		"GloryRenderer",
		"GlorySDLWindow",
		"GlorySDLInput",
		"GloryEditorSDL",
		"GloryEditorOGL",
		"GloryJoltPhysics",
		"GlorySteamAudio",
		"GloryEditorVulkan",
		"GloryMonoScripting",
		"GloryUIRenderer",
		"GloryFSM",
		"GloryLocalize",
		"GloryOverlayConsole",
		"SDLAudioExtension",
		"EntityScenesEditorExtension",
		"JoltEditorExtension",
		"MonoEditorExtension",
		"FreeTypeFontImporter",
		"UIEditorExtension",
		"FSMEditorExtension",
		"LocalizeEditorExtension",
		"SDLImageImporter",
		"STBImageImporter",
		"ASSIMPImporter",
		"GloryJoltMonoExtender",
		"GloryUIMonoExtender",
		"GloryFSMMonoExtender",
		"GloryLocalizeMonoExtender",
		"SteamAudioExtension",
		"GloryEngine.Core",
		"GloryEngine.Jolt",
		"GloryEngine.UI",
		"GloryEngine.FSM",
		"GloryEngine.Localize",
		"CSAPIInstall",
		"GloryRuntime",
		"GloryRuntimeApplication",
		"WriteVersion",
	}

	postbuildcommands
	{
		("{COPY} %{modulesOutDir} %{editorBuildDir}/Modules"),
		("{COPY} %{engineOutDir}/GloryEditor.dll %{editorBuildDir}"),
		("{COPY} %{engineOutDir}/Extensions %{editorBuildDir}/Extensions"),
		("{COPY} %{engineOutDir}/version.txt %{editorBuildDir}"),
		("{COPY} %{engineOutDir}/Patchnotes %{editorBuildDir}/Patchnotes"),
		("{COPY} %{engineOutDir}/GloryAPI.dll %{editorBuildDir}"),
		("{COPY} %{engineOutDir}/libcurl*.dll %{editorBuildDir}"),
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

		("{COPY} %{engineOutDir}/Splash.bmp %{engineOutDir}/EditorAssets/Runtime"),
		("{COPY} %{engineOutDir}/Splash.bmp %{editorBuildDir}/EditorAssets/Runtime"),
		("{COPY} %{engineOutDir}/GloryRuntime.dll %{editorBuildDir}/EditorAssets/Runtime"),
		("{COPY} %{engineOutDir}/GloryRuntime.dll %{engineOutDir}/EditorAssets/Runtime"),
		("{COPY} %{engineOutDir}/GloryRuntimeApplication.dll %{editorBuildDir}/EditorAssets/Runtime"),
		("{COPY} %{engineOutDir}/GloryRuntimeApplication.dll %{engineOutDir}/EditorAssets/Runtime"),
		("{COPY} %{GloryIncludeDir.runtimeTestApp}/RuntimeAPI.h %{editorBuildDir}/EditorAssets/Runtime"),
		("{COPY} %{GloryIncludeDir.runtimeTestApp}/RuntimeAPI.h %{engineOutDir}/EditorAssets/Runtime"),
		("{COPY} %{GloryIncludeDir.runtimeTestApp}/Application.cpp %{editorBuildDir}/EditorAssets/Runtime"),
		("{COPY} %{GloryIncludeDir.runtimeTestApp}/Application.cpp %{engineOutDir}/EditorAssets/Runtime"),
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
