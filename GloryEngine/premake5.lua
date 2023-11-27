workspace "GloryEngine"
	platforms
	{
		"Win32",
		"x64"
	}

	startproject "GloryCore"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

vulkanDir								= "C:/VulkanSDK/1.2.182.0"
monoDir									= "C:/Program Files/Mono"
leakDetectorDir							= "C:/Program Files (x86)/Visual Leak Detector/include"

rootDir									= "%{wks.location}"
buildDir								= "%{rootDir}/Build/%{cfg.buildcfg}/%{cfg.platform}"
engineDir								= "%{rootDir}/Engine"
editorDir								= "%{rootDir}/Editor"
modulesDir								= "%{rootDir}/Modules"
engineOutDir							= "%{rootDir}/bin/Engine/%{cfg.buildcfg}/%{cfg.platform}"
modulesOutDir							= "%{engineOutDir}/Modules"
moduleOutDir							= "%{modulesOutDir}/%{prj.name}"

outputDir								= "%{cfg.buildcfg}/%{cfg.platform}"

premakeDir								= "%{rootDir}/third-party/premake"
demosDir								= "%{rootDir}/Demos"
editorBuildDir							= "%{buildDir}/Editor"

GloryIncludeDir = {}
GloryIncludeDir["assimp"]				= "%{modulesDir}/GloryASSIMPModelLoader"
GloryIncludeDir["clusteredrenderer"]	= "%{modulesDir}/GloryClusteredRenderer"
GloryIncludeDir["entityscenes"]			= "%{modulesDir}/GloryEntityScenes"
GloryIncludeDir["opengl"]				= "%{modulesDir}/GloryOpenGLGraphics"
GloryIncludeDir["sdlimage"]				= "%{modulesDir}/GlorySDLImage"
GloryIncludeDir["sdlwindow"]			= "%{modulesDir}/GlorySDLWindow"
GloryIncludeDir["vulkan"]				= "%{modulesDir}/GloryVulkanGraphics"
GloryIncludeDir["mono"]					= "%{modulesDir}/GloryMonoScripting"
GloryIncludeDir["jolt"]					= "%{modulesDir}/GloryJoltPhysics"

GloryIncludeDir["core"]					= "%{engineDir}/Core"
GloryIncludeDir["editor"]				= "%{editorDir}/GloryEditor"
GloryIncludeDir["api"]					= "%{editorDir}/GloryAPI"
GloryIncludeDir["jobs"]					= "%{engineDir}/Jobs"
GloryIncludeDir["threads"]				= "%{engineDir}/Threads"

SubmodoleDirs = {}
SubmodoleDirs["ImGui"]					= "%{rootDir}/submodules/ImGui"
SubmodoleDirs["implot"]					= "%{rootDir}/submodules/implot"
SubmodoleDirs["ImGuizmo"]				= "%{rootDir}/submodules/ImGuizmo"
SubmodoleDirs["yaml_cpp"]				= "%{rootDir}/submodules/yaml-cpp"
SubmodoleDirs["glm"]					= "%{rootDir}/submodules/glm"

IncludeDir = {}
IncludeDir["glm"]						= "%{SubmodoleDirs.glm}"
IncludeDir["ImGui"]						= "%{SubmodoleDirs.ImGui}"
IncludeDir["implot"]					= "%{SubmodoleDirs.implot}"
IncludeDir["ImGuizmo"]					= "%{SubmodoleDirs.ImGuizmo}"
IncludeDir["yaml_cpp"]					= "%{SubmodoleDirs.yaml_cpp}/include"
IncludeDir["rapidjson"]					= "%{SubmodoleDirs.rapidjson}/include"
IncludeDir["Utils"]						= "%{rootDir}/Utils"
IncludeDir["ECS"]						= "%{IncludeDir.Utils}/GloryECS"
IncludeDir["Reflect"]					= "%{IncludeDir.Utils}/GloryReflect"
IncludeDir["Version"]					= "%{IncludeDir.Utils}/Version"

IncludeDir["FA"]						= "%{rootDir}/third-party/FontAwesome"

LibDirs = {}
LibDirs["glory"]						= "%{engineOutDir}/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGui"]						= "%{SubmodoleDirs.ImGui}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["ImGuizmo"]						= "%{SubmodoleDirs.ImGuizmo}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["implot"]						= "%{SubmodoleDirs.implot}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["yaml_cpp"]						= "%{SubmodoleDirs.yaml_cpp}/lib/%{cfg.buildcfg}/%{cfg.platform}"
LibDirs["extensions"]					= "%{SubmodoleDirs.glory}/%{cfg.buildcfg}/%{cfg.platform}/Extensions"

stb_image								= "%{rootDir}/third-party/stb_image"

-- External dependencies
DepsRootDir								= "%{rootDir}/Dependencies/%{cfg.buildcfg}"
DepsIncludeDir							= "%{DepsRootDir}/include"
DepsBinDir								= "%{DepsRootDir}/bin"
DepsLibDir								= "%{DepsRootDir}/lib"
rapidjson								= "%{rootDir}/submodules/rapidjson/include"

group "Dependencies"
	include "third-party/ImGui"
	include "third-party/ImGuizmo"
	include "third-party/implot"
	include "third-party/yaml-cpp"
	include "third-party/tinyfiledialogs"
group ""

group "Utils"
	include "Utils"
	include "Utils/GloryECS"
	include "Utils/GloryReflect"
	include "Utils/Version"
group ""

group "Engine"
	include "Engine/GloryEngineBuilder"
	include "Engine/Core"
	include "Engine/Threads"
	include "Engine/Jobs"
group ""

group "Modules"
	include "Modules/GloryASSIMPModelLoader"
	include "Modules/GloryClusteredRenderer"
	include "Modules/GloryOpenGLGraphics"
	include "Modules/GlorySDLImage"
	include "Modules/GlorySDLWindow"
	include "Modules/GloryMonoScripting"
	include "Modules/GlorySDLInput"
	include "Modules/GloryJoltPhysics"
	--include "GloryVulkanGraphics"
group ""

group "Editor"
	include "Editor/GloryEditor"
	include "Editor/GloryAPI"
	include "Editor/WriteVersion"
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
	include "Editor/Extensions/MonoEditorExtension"
	include "Editor/Extensions/JoltEditorExtension"
	include "Editor/Extensions/Importers/SDLImageImporter"
	include "Editor/Extensions/Importers/ASSIMPImporter"
	include "Editor/Extensions/Importers/EntityPrefabImporter"
group ""

group "Scripting/Mono"
	include "Scripting/Mono/GloryEngine.Core"
	include "Scripting/Mono/GloryEngine.Jolt"
	include "Scripting/Mono/GloryJoltMonoExtender"
	include "Scripting/Mono/Install"
group ""