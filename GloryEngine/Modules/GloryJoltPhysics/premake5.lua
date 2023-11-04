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
		["Module"] = { "GloryJoltPhysics.*", "JoltPhysicsModule.*", "JoltCharacterManager.*", "JoltShapeManager.*" },
		["Collision"] = { "BroadPhaseImpl.*", "LayerCollisionFilter.*", "JoltDebugRenderer.*" },
		["Helpers"] = { "Helpers.*", "ShapeHandlers.*" }
	}

	includedirs
	{
		"%{DepsIncludeDir}",
		"%{IncludeDir.glm}",

		"%{GloryIncludeDir.core}",
		"%{GloryIncludeDir.entityscenes}",

		"%{IncludeDir.ECS}",
		"%{IncludeDir.Reflect}",
		"%{IncludeDir.Version}",
		"%{IncludeDir.ECS}",
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



		--temporary until entity scenes module becomes internal
		"%{modulesDir}/GloryEntityScenes",
	}

	links
	{
		"GloryCore",
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


		--temporary until entity scenes module becomes internal
		"GloryEntityScenes",
	}

	defines
	{
		"GLORY_EXPORTS",
		"JPH_DEBUG_RENDERER"
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
		defines "_DEBUG"
		symbols "On"

		links "SDL2d"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"

		links "SDL2"
