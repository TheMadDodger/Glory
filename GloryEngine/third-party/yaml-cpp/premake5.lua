YAMLDir = "%{SubmodoleDirs.yaml_cpp}"

os.copyfile("dll.h", "%{YAMLDir}/include/yaml-cpp/dll.h")

project "yaml-cpp"
	location "%{YAMLDir}"
	kind "StaticLib"
	language "C++"

	targetdir ("$(ProjectDir)/lib/%{outputDir}")
	objdir ("$(ProjectDir)/%{outputDir}")

	files
	{
		"%{YAMLDir}/include/**.h",
        "%{YAMLDir}/src/**.h",
        "%{YAMLDir}/src/**.cpp",
	}

	sysincludedirs
    {
        "%{YAMLDir}/include",
    }
        
    includedirs
    {
        "%{YAMLDir}/include",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"yaml_cpp_EXPORTS",
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "NDEBUG"
		optimize "On"
