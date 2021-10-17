workspace "GloryEngine"
	platforms {
		"x86",
		"x64"
	}

	startproject "GloryASSIMPModelLoader"
	startproject "GloryBasicRenderer"
	startproject "GloryCore"
	startproject "GloryEditor"
	startproject "GloryEntityScenes"
	startproject "GloryGLSLShaderLoader"
	startproject "GloryMain"
	startproject "GloryOpenGLGraphics"
	startproject "GlorySDLImage"
	startproject "GlorySDLWindow"
	startproject "GloryVulkanGraphics"
	startproject "ImGui"
	
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

IncludeDir = {}
GloryIncludeDir["assimp"]			= "../GloryASSIMPModelLoader"
GloryIncludeDir["basicrenderer"]	= "../GloryBasicRenderer"
GloryIncludeDir["core"]				= "../GloryCore"
GloryIncludeDir["editor"]			= "../GloryEditor"
GloryIncludeDir["entityscenes"]		= "../GloryEntityScenes"
GloryIncludeDir["glslloader"]		= "../GloryGLSLShaderLoader"
GloryIncludeDir["main"]				= "../GloryMain"
GloryIncludeDir["opengl"]			= "../GloryOpenGLGraphics"
GloryIncludeDir["sdlimage"]			= "../GlorySDLImage"
GloryIncludeDir["sdlwindow"]		= "../GlorySDLWindow"
GloryIncludeDir["vulkan"]			= "../GloryVulkanGraphics"
GloryIncludeDir["ImGui"]			= "../ImGui"

SubmodoleDirs = {}
SubmodoleDirs["assimp"]			= "../bin/yaml-cpp"
SubmodoleDirs["glory"]			= "../bin/Engine"
SubmodoleDirs["GLEW"]			= "../bin/GLEW"
SubmodoleDirs["ImGui"]			= "../bin/ImGui"
SubmodoleDirs["SDL_Image"]		= "../bin/SDL_Image"
SubmodoleDirs["shaderc"]		= "../bin/shaderc"
SubmodoleDirs["spirv_cross"]	= "../bin/spirv-cross"
SubmodoleDirs["yaml_cpp"]		= "../bin/yaml-cpp"

IncludeDir["assimp"]			= "%{SubmodoleDirs.assimp}/include"
IncludeDir["glory"]				= "%{SubmodoleDirs.glory}/include"
IncludeDir["GLEW"]				= "%{SubmodoleDirs.GLEW}/include"
IncludeDir["ImGui"]				= "%{SubmodoleDirs.ImGui}/include"
IncludeDir["SDL_Image"]			= "%{SubmodoleDirs.SDL_Image}/include"
IncludeDir["shaderc"]			= "%{SubmodoleDirs.shaderc}/include"
IncludeDir["spirv_cross"]		= "%{SubmodoleDirs.spirv_cross}/include"
IncludeDir["yaml_cpp"]			= "%{SubmodoleDirs.yaml_cpp}/include"

group "Dependencies"
	
group ""

include "GloryASSIMPModelLoader"
include "GloryBasicRenderer"
include "GloryCore"
include "GloryEditor"
include "GloryEntityScenes"
include "GloryGLSLShaderLoader"
include "GloryMain"
include "GloryOpenGLGraphics"
include "GlorySDLImage"
include "GlorySDLWindow"
include "GloryVulkanGraphics"
include "ImGui"