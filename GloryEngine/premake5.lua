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
outputdir = "%{cfg.buildcfg}/%{cfg.platform}"
engineoutdir = "../bin/Engine/%{cfg.buildcfg}/%{cfg.platform}"

SubmodoleDirs = {}
SubmodoleDirs["yaml_cpp"] = "submodules/yaml-cpp"

IncludeDir = {}
IncludeDir["glorycore"] = "../GloryCore"
IncludeDir["yaml_cpp"] = "%{SubmodoleDirs.yaml_cpp}/include"

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