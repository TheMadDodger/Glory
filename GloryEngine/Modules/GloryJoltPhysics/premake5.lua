project "GloryJoltPhysics"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{moduleOutDir}")
	objdir ("%{outputDir}")

	files
	{
		"**.h",
		"**.cpp",
		"Module.yaml",
		"premake5.lua",

		"%{rootDir}/submodules/JoltPhysics/Jolt/**.*",
	}

	vpaths
	{
		["Module"] = { "Physics.*", "GloryJoltPhysics.*", "JoltPhysicsModule.*", "JoltCharacterManager.*", "JoltShapeManager.*" },
		["Collision"] = { "BroadPhaseImpl.*", "LayerCollisionFilter.*", "JoltDebugRenderer.*" },
		["Helpers"] = { "Helpers.*", "ShapeHandlers.*" },
		["ECS"] = { "CharacterControllerSystem.*", "JoltComponents.*", "PhysicsSystem.*" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",
		"%{IncludeDir.glm}",
		"%{BaseIncludeDir.physics}",

		"%{GloryIncludeDir.core}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.Utils}",

		"%{IncludeDir.yaml_cpp}",

		"%{DepIncludesDir}",
	}

	sysincludedirs
	{
		"%{rootDir}/submodules/JoltPhysics",
	}

	libdirs
	{
		"%{DepsLibDir}",

		"%{LibDirs.glory}",

		"%{LibDirs.yaml_cpp}",

		"{moduleBaseOutDir}",
	}

	links
	{
		"GloryCore",
		"GloryPhysicsModule",
		"GloryReflect",
		"GloryECS",

		"shaderc",
		"shaderc_combined",
		"shaderc_shared",
		"yaml-cpp",

		"GloryUtils",
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
	}

	postbuildcommands
	{
		("{COPY} ./Module.yaml %{moduleOutDir}"),
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
		defines
		{
			"_DEBUG",
			"JPH_DEBUG_RENDERER"
		}
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
