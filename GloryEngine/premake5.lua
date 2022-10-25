workspace "GloryEngine"
	platforms {
		"Win32",
		"x64"
	}

	startproject "GloryASSIMPModelLoader"
	startproject "GloryBasicRenderer"
	startproject "GloryClusteredRenderer"
	startproject "GloryCore"
	startproject "GloryEditor"
	startproject "GloryEntityScenes"
	startproject "GloryEntityScenes2"
	startproject "GloryOpenGLGraphics"
	startproject "GlorySDLImage"
	startproject "GlorySDLWindow"
	startproject "GloryVulkanGraphics"
	startproject "Glorious"
	startproject "GloriousLauncher"
	startproject "GloryEngineBuilder"
	startproject "GloryEditorSDL"
	startproject "GloryEditorOGL"
	startproject "GloryEditorVulkan"
	
	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

vulkan_sdk = "C:/VulkanSDK/1.2.182.0"
leak_detector = "C:/Program Files (x86)/Visual Leak Detector/include"
outputdir = "%{cfg.buildcfg}/%{cfg.platform}"
engineoutdir = "../bin/Engine/%{cfg.buildcfg}/%{cfg.platform}"
editorOutDir = "%{engineoutdir}/Editor"
modulesOutDir = "%{engineoutdir}/Modules"
moduleOutDir = "%{modulesOutDir}/%{prj.name}"

GloryIncludeDir = {}
GloryIncludeDir["assimp"]				= "../GloryASSIMPModelLoader"
GloryIncludeDir["basicrenderer"]		= "../GloryBasicRenderer"
GloryIncludeDir["clusteredrenderer"]	= "../GloryClusteredRenderer"
GloryIncludeDir["core"]					= "../GloryCore"
GloryIncludeDir["editor"]				= "../GloryEditor"
GloryIncludeDir["entityscenes"]			= "../GloryEntityScenes"
GloryIncludeDir["entityscenes2"]		= "../GloryEntityScenes2"
GloryIncludeDir["main"]					= "../GloryMain"
GloryIncludeDir["opengl"]				= "../GloryOpenGLGraphics"
GloryIncludeDir["sdlimage"]				= "../GlorySDLImage"
GloryIncludeDir["sdlwindow"]			= "../GlorySDLWindow"
GloryIncludeDir["vulkan"]				= "../GloryVulkanGraphics"
GloryIncludeDir["ImGui"]				= "../ImGui"
GloryIncludeDir["ImGuizmo"]				= "../ImGuizmo"
GloryIncludeDir["implot"]				= "../implot"
GloryIncludeDir["ImFileDialog"]			= "../ImFileDialog"

GloryIncludeDir["ECS"]					= "C:/Users/Sparta/Documents/GloryECS/GloryECS/GloryECSStatic"
GloryIncludeDir["Reflect"]				= "C:/Users/Sparta/Documents/GloryECS/GloryECS/GloryReflection"

SubmodoleDirs = {}
SubmodoleDirs["assimp"]				= "../submodules/assimp"
SubmodoleDirs["ImGui"]				= "../submodules/ImGui"
SubmodoleDirs["ImGuizmo"]			= "../submodules/ImGuizmo"
SubmodoleDirs["implot"]				= "../submodules/implot"
SubmodoleDirs["ImFileDialog"]		= "../submodules/ImFileDialog"
SubmodoleDirs["yaml_cpp"]			= "../submodules/yaml-cpp"
SubmodoleDirs["GLEW"]				= "../third-party/GLEW"
SubmodoleDirs["SDL_image"]			= "../third-party/SDL_Image"
SubmodoleDirs["shaderc"]			= "../third-party/shaderc"
SubmodoleDirs["spirv_cross"]		= "../third-party/spirv-cross"
SubmodoleDirs["glory"]				= "../bin/Engine"

IncludeDir = {}
IncludeDir["assimp"]				= "%{SubmodoleDirs.assimp}/include"
IncludeDir["glory"]					= "%{SubmodoleDirs.glory}/include"
IncludeDir["GLEW"]					= "%{SubmodoleDirs.GLEW}/include"
IncludeDir["ImGui"]					= "%{SubmodoleDirs.ImGui}"
IncludeDir["ImGuizmo"]				= "%{SubmodoleDirs.ImGuizmo}"
IncludeDir["implot"]				= "%{SubmodoleDirs.implot}"
IncludeDir["ImFileDialog"]			= "%{SubmodoleDirs.ImFileDialog}"
IncludeDir["SDL_image"]				= "%{SubmodoleDirs.SDL_image}/include"
IncludeDir["shaderc"]				= "%{SubmodoleDirs.shaderc}/include"
IncludeDir["spirv_cross"]			= "%{SubmodoleDirs.spirv_cross}/include"
IncludeDir["yaml_cpp"]				= "%{SubmodoleDirs.yaml_cpp}/include"

LibDirs = {}
LibDirs["assimp"]					= "%{SubmodoleDirs.assimp}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["glory"]					= "%{SubmodoleDirs.glory}/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["GLEW"]						= "%{SubmodoleDirs.GLEW}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGui"]					= "%{SubmodoleDirs.ImGui}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGuizmo"]					= "%{SubmodoleDirs.ImGuizmo}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["implot"]					= "%{SubmodoleDirs.implot}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImFileDialog"]				= "%{SubmodoleDirs.ImFileDialog}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["SDL_image"]				= "%{SubmodoleDirs.SDL_image}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["shaderc"]					= "%{SubmodoleDirs.shaderc}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["spirv_cross"]				= "%{SubmodoleDirs.spirv_cross}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["yaml_cpp"]					= "%{SubmodoleDirs.yaml_cpp}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["extensions"]				= "%{SubmodoleDirs.glory}/%{cfg.buildcfg}/%{cfg.platform}/Extensions"

LibDirs["ECS"]						= "C:/Users/Sparta/Documents/GloryECS/GloryECS/Debug"
LibDirs["Reflect"]					= "C:/Users/Sparta/Documents/GloryECS/GloryECS/Debug"

stb_image							= "../../third-party/stb_image"

group "Dependencies"
	include "third-party/ImGui"
	include "third-party/ImGuizmo"
	include "third-party/implot"
	include "submodules/ImFileDialog"
	include "third-party/assimp"
	include "third-party/yaml-cpp"
	include "submodules/GloryECS/GloryECS/GloryECSStatic"
	include "submodules/GloryECS/GloryECS/GloryReflectStatic"
group ""

include "GloryASSIMPModelLoader"
include "GloryBasicRenderer"
include "GloryClusteredRenderer"
include "GloryCore"
include "GloryEditor"
include "GloryEntityScenes"
include "GloryEntityScenes2"
include "GloryOpenGLGraphics"
include "GlorySDLImage"
include "GlorySDLWindow"
include "GloryVulkanGraphics"

group "Application"
	include "GloryEngineBuilder"
	include "Glorious"
	include "GloriousLauncher"
group ""

group "Editor Backends"
	include "GloryEditorSDL"
	include "GloryEditorOGL"
	include "GloryEditorVulkan"
group ""

group "Editor Extensions"
	include "EntityScenesEditorExtension"
group ""