# Glory Engine
## A modular game engine and editor.

Glory is an under development, modular game engine and editor, written in C++.
It has a detailed [profiler](https://github.com/TheMadDodger/Glory/wiki/Profiling), comes with a clustered renderer supporting 3000+ dynamic lights, is fully customizable and is platform independant thanks to its modularity.

![Glory Editor](https://glory-engine.com/static/media/Editor.192132f3ded68e7992eb.png)

## Features
- Extremely expandable due to modular design
- Customizable editor made using ImGui
- Scripting in any language your hearth desires
- Write custom modules
- Built-in asset management
- Use any graphics API
- Project HUB
- Runtime shader compilation and in the future cross platform shader compilation
- Built-in [profiler](https://github.com/TheMadDodger/Glory/wiki/Profiling)
- [Launcher/Hub](https://github.com/TheMadDodger/Glory/wiki/Editor-Launcher-Hub) with support for multiple editor installations (similar to Unity)
- Built-in version checking
- Entity Component System

## Built-in Modules
- Clustered Renderer
- OpenGL Graphics API
- Vulkan Graphics API
- ECS Scene System
- Texture loading using SDL image
- SDL Window
- Physics engine using the Jolt Physics library
- Scripting in C# using mono
- Input processing with SDL
- SDL Mixer Audio
- Steam Audio (3D audio simulation)
- Finite State Machines module
- UI Renderer
- Localization module
- Overlay console

## Built-in editor extensions
- ASSIMP importer
- Font importer based on FreeType2
- FSM editor
- Audio importer
- Jolt Physics editor
- Localization editor
- Mono Scripting editor
- SDL Audio editor
- SDL Image importer
- HDRI importer and cubemap generator using stb_image and HdriToCubemap
- Steam Audio editor

## Tech

Glory uses the following libraries:

- [Dear ImGui](https://github.com/ocornut/imgui) - Open-source GUI library used by the editor.
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) - Amazing library for 3D gizmos and grids rendered using ImGui.
- [ImPlot](https://github.com/epezent/implot) - Epic library for plotting graphs in ImGui.
- [RapidJSON](https://github.com/Tencent/rapidjson/) - The fastest json parser and writer in existance.
- [ASSIMP](http://assimp.org/) - Open asset import library for importing 3D files like FBX, OBJ, CAD and many more.
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML file parser for C++.
- [SDL](https://www.libsdl.org/) - Simple Direct Media Layer library, used for opening a window.
- [SDL_Image](https://github.com/libsdl-org/SDL_image) - Image loading library for SDL.
- [SDL_mixer](https://github.com/libsdl-org/SDL_mixer) - An audio management library.
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) - The future of computer graphics.
- [GLEW](http://glew.sourceforge.net/) - OpenGL extension wrangler library.
- [GLM](https://github.com/g-truc/glm) - Mathematics library for graphics software.
- [Premake5](https://premake.github.io/) - Powerfully simple build configuration.
- [shaderc](https://github.com/google/shaderc) - A collection of tools, libraries and tests for shader compilation.
- [SPIRV-cross](https://github.com/KhronosGroup/SPIRV-Cross) - Tool designed for parsing and converting SPIR-V to other shader languages.
- [SPIRV-tools](https://github.com/KhronosGroup/SPIRV-Tools) - Tool that provides an API and commands for processing SPIR-V modules.
- [glslang](https://github.com/KhronosGroup/glslang)
- [Mono](https://www.mono-project.com/) - Software platform designed to allow developers to easily create cross platform applications.
- [Jolt Physics](https://github.com/jrouwe/JoltPhysics) - A multi core friendly rigid body physics and collision detection library suitable for games and VR applications, used by Horizon Forbidden West.
- [curl](https://curl.se/) - Command line tool and library for transferring data with URLs (since 1998)
- [stb_image.h](https://github.com/nothings/stb) - Single-file public domain (or MIT licensed) libraries for C/C++.
- [esfw](https://github.com/SpartanJ/efsw) - Simple and fast cross platform filewatch.
- [tinyfiledialogs](https://github.com/native-toolkit/libtinyfiledialogs) - Simple and fast cross platform file dialog.
- [FontAwesome](https://fontawesome.com/) - Large collection of icons contained in a font.
- [zlib](https://www.zlib.net/) - A Massively Spiffy Yet Delicately Unobtrusive Compression Library.
- [HdriToCubemap](https://github.com/ivarout/HdriToCubemap) - A simple C++ library to convert hdri (equirectangular) environment maps to cubemaps.
- [SteamAudio](https://github.com/ValveSoftware/steam-audio) - A 3D audio simulation engine made by Valve.
- [CRCpp](https://github.com/d-bahr/CRCpp) - A fast C++ CRC calculator.
- [FreeType](https://github.com/freetype/freetype) - A freely available software library to render fonts.

## Installation

### Install Dependencies

Make sure to install Vulkan SDK 1.4.321.1 [here](https://vulkan.lunarg.com/sdk/home), and the latest version of [mono](https://www.mono-project.com/download/stable/).
Also make sure to install [.NET Framework 4.7.2](https://dotnet.microsoft.com/en-us/download/dotnet-framework/net472).
To build and install dependencies, run the install-dependencies.sh script using the following command line:
`./install-dependencies.sh [configuration]`
There are currently 2 options for configuration, `Debug` and `Release`.

### Build the engine

Run the `generateprojects.bat` file to run the premake scripts and generate the visual studio project files.
Open `GloryEngine.sln` and build the whole solution.
Run the editor by running Glorious.

Run the luncher/hub by running GloriousLauncher.

Run the runtime by running RuntimeTestApplication, make sure to package the Sponza demo through the editor before running this.

## License

The current version of Glory is under a `Personal Use Only` license.
This will change in the near future to allow commercial usage under limitations.
