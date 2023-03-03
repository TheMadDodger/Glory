workspace "GloryEngine"
	platforms
	{
		"Win32",
		"x64"
	}

	startproject "GloryASSIMPModelLoader"
	startproject "GloryClusteredRenderer"
	startproject "GloryCore"
	startproject "GloryEditor"
	startproject "GloryEntityScenes"
	startproject "GloryOpenGLGraphics"
	startproject "GlorySDLImage"
	startproject "GlorySDLWindow"
	startproject "Glorious"
	startproject "GloriousLauncher"
	startproject "GloryEngineBuilder"
	startproject "GloryEditorSDL"
	startproject "GloryEditorOGL"
	startproject "GloryMonoScripting"
	startproject "GlorySDLInput"
	--startproject "GloryEditorVulkan"
	--startproject "GloryVulkanGraphics"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

vulkanDir = "C:/VulkanSDK/1.2.182.0"
monoDir = "C:/Program Files/Mono"
monox86Dir = "C:/Program Files (x86)/Mono"
leakDetectorDir = "C:/Program Files (x86)/Visual Leak Detector/include"

rootDir = "%{wks.location}"
buildDir = "%{rootDir}/Build/%{cfg.buildcfg}/%{cfg.platform}"
engineDir = "%{rootDir}/Engine"
editorDir = "%{rootDir}/Editor"
modulesDir = "%{engineDir}/Modules"
engineOutDir = "%{rootDir}/bin/Engine/%{cfg.buildcfg}/%{cfg.platform}"
modulesOutDir = "%{engineOutDir}/Modules"
moduleOutDir = "%{modulesOutDir}/%{prj.name}"

outputDir = "%{cfg.buildcfg}/%{cfg.platform}"

premakeDir = "%{rootDir}/third-party/premake"
demosDir = "%{rootDir}/Demos"
editorBuildDir = "%{buildDir}/Editor"

GloryIncludeDir = {}
GloryIncludeDir["assimp"]				= "%{modulesDir}/GloryASSIMPModelLoader"
GloryIncludeDir["clusteredrenderer"]	= "%{modulesDir}/GloryClusteredRenderer"
GloryIncludeDir["entityscenes"]			= "%{modulesDir}/GloryEntityScenes"
GloryIncludeDir["opengl"]				= "%{modulesDir}/GloryOpenGLGraphics"
GloryIncludeDir["sdlimage"]				= "%{modulesDir}/GlorySDLImage"
GloryIncludeDir["sdlwindow"]			= "%{modulesDir}/GlorySDLWindow"
GloryIncludeDir["vulkan"]				= "%{modulesDir}/GloryVulkanGraphics"
GloryIncludeDir["mono"]					= "%{modulesDir}/GloryMonoScripting"

GloryIncludeDir["core"]					= "%{engineDir}/GloryCore"
GloryIncludeDir["editor"]				= "%{editorDir}/GloryEditor"

SubmodoleDirs = {}
SubmodoleDirs["ImGui"]				= "%{rootDir}/submodules/ImGui"
SubmodoleDirs["implot"]				= "%{rootDir}/submodules/implot"
SubmodoleDirs["ImGuizmo"]			= "%{rootDir}/submodules/ImGuizmo"
SubmodoleDirs["ImFileDialog"]		= "%{rootDir}/submodules/ImFileDialog"
SubmodoleDirs["yaml_cpp"]			= "%{rootDir}/submodules/yaml-cpp"
SubmodoleDirs["glm"]				= "%{rootDir}/submodules/glm"
SubmodoleDirs["GloryECS"]			= "%{rootDir}/submodules/GloryECS/GloryECS"
SubmodoleDirs["ECS"]				= "%{SubmodoleDirs.GloryECS}/GloryECSStatic"
SubmodoleDirs["Reflect"]			= "%{SubmodoleDirs.GloryECS}/GloryReflectStatic"

IncludeDir = {}
IncludeDir["glm"]					= "%{SubmodoleDirs.glm}"
IncludeDir["ImGui"]					= "%{SubmodoleDirs.ImGui}"
IncludeDir["implot"]				= "%{SubmodoleDirs.implot}"
IncludeDir["ImGuizmo"]				= "%{SubmodoleDirs.ImGuizmo}"
IncludeDir["ImFileDialog"]			= "%{SubmodoleDirs.ImFileDialog}"
IncludeDir["yaml_cpp"]				= "%{SubmodoleDirs.yaml_cpp}/include"
IncludeDir["ECS"]					= "%{SubmodoleDirs.ECS}"
IncludeDir["Reflect"]				= "%{SubmodoleDirs.Reflect}"

IncludeDir["FA"]					= "%{rootDir}/third-party/FontAwesome"

LibDirs = {}
LibDirs["glory"]					= "%{engineOutDir}/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGui"]					= "%{SubmodoleDirs.ImGui}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGuizmo"]					= "%{SubmodoleDirs.ImGuizmo}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["implot"]					= "%{SubmodoleDirs.implot}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImFileDialog"]				= "%{SubmodoleDirs.ImFileDialog}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["yaml_cpp"]					= "%{SubmodoleDirs.yaml_cpp}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["extensions"]				= "%{SubmodoleDirs.glory}/%{cfg.buildcfg}/%{cfg.platform}/Extensions"
LibDirs["GloryECS"]					= "%{SubmodoleDirs.GloryECS}/Build/Lib/%{cfg.buildcfg}/%{cfg.platform}/Extensions"

stb_image							= "%{rootDir}/third-party/stb_image"

-- External dependencies
DepsRootDir							= "%{rootDir}/Dependencies/%{cfg.buildcfg}"
DepsIncludeDir						= "%{DepsRootDir}/include"
DepsBinDir							= "%{DepsRootDir}/bin"
DepsLibDir							= "%{DepsRootDir}/lib"

group "Dependencies"
	include "third-party/ImGui"
	include "third-party/ImGuizmo"
	include "third-party/implot"
	include "submodules/ImFileDialog"
	include "third-party/yaml-cpp"
	include "submodules/GloryECS/GloryECS/GloryECSStatic"
	include "submodules/GloryECS/GloryECS/GloryReflectStatic"
group ""

group "Engine"
	include "Engine/GloryEngineBuilder"
	include "Engine/GloryCore"
group ""

group "Engine/Modules"
	include "Engine/Modules/GloryASSIMPModelLoader"
	include "Engine/Modules/GloryClusteredRenderer"
	include "Engine/Modules/GloryEntityScenes"
	include "Engine/Modules/GloryOpenGLGraphics"
	include "Engine/Modules/GlorySDLImage"
	include "Engine/Modules/GlorySDLWindow"
	include "Engine/Modules/GloryMonoScripting"
	include "Engine/Modules/GlorySDLInput"
	--include "GloryVulkanGraphics"
group ""

group "Editor"
	include "Editor/GloryEditor"
group ""

group "Editor/Applications"
	include "Editor/Applications/Glorious"
	include "Editor/Applications/GloriousLauncher"
group ""

group "Editor/Backend"
	include "Editor/Backend/GloryEditorSDL"
	include "Editor/Backend/GloryEditorOGL"
	--include "GloryEditorVulkan"
group ""

group "Editor/Extensions"
	include "Editor/Extensions/EntityScenesEditorExtension"
	include "Editor/Extensions/MonoEditorExtension"
group ""

group "Scripting/Mono"
	include "Scripting/Mono/GloryEngine.Core"
	include "Scripting/Mono/GloryEngine.Entities"
	include "Scripting/Mono/GloryEntitiesMonoExtender"
	include "Scripting/Mono/Install"
group ""