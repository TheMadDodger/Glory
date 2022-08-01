# Glory Engine
## A modular game engine and editor.

Glory is an under development, modular game engine and editor, written in C++.
Many features are still missing, and it is not yet in a usable state, but it is getting there.
If you want the most up-to-date development code for the engine, head on to the [dynamic-modules](https://github.com/TheMadDodger/Glory/tree/dynamic_modules) branch for dynamic module loading features, or to the [scripting/csharp](https://github.com/TheMadDodger/Glory/tree/scripting/csharp) branch for the scripting implementation. They should get merged relatively soon. It should be noted that these branches will likely not build for you.

![Glory Editor](https://i.imgur.com/5YlgbGP.png)

## Features

- Extremely expandable due to modular design
- Customizable editor made using ImGui
- Scripting in any language your hearth desires (WIP on scripting branch)
- Write custom modules
- Built-in asset management
- Use any graphics API
- Project HUB
- Runtime shader compilation and in the future cross platform shader compilation

## Built-in Modules
- Clustered Renderer
- OpenGL Graphics API
- Model loader using ASSIMP
- ECS Scene System
- Texture loading using SDL image
- SDL Window
- Vulkan Graphics API (WIP)

## Tech

Glory uses the following libraries:

- [Dear ImGui](https://github.com/ocornut/imgui) - Open-source GUI library used by the editor.
- [ImFileDialog](https://github.com/dfranx/ImFileDialog) - An awesome file dialog widget rendered using ImGui.
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) - Amazing library for 3D gizmos and grids rendered using ImGui.
- [implot](https://github.com/epezent/implot) - Epic library for plotting graphs in ImGui.
- [ASSIMP](https://github.com/assimp/assimp) - Open asset import library for importing 3D files like FBX, OBJ, CAD and many more.
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - Extremely uses YAML file parser for C++.
- [SDL](https://www.libsdl.org/) - Simple Direct Media Layer library, used for opening a window.
- [SDL_Image](https://github.com/libsdl-org/SDL_image) - Image loading library for SDL.
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) - The future of computer graphics.
- [GLEW](http://glew.sourceforge.net/) - OpenGL extension wrangler library.
- [GLM](https://github.com/g-truc/glm) - Mathematics library for graphics software.
- [Premake5](https://premake.github.io/) - Powerfully simple build configuration.
- [shaderc](https://github.com/google/shaderc) - A collection of tools, libraries and tests for shader compilation.
- [spirv-cross](https://github.com/KhronosGroup/SPIRV-Cross) - Tool designed for parsing and converting SPIR-V to other shader languages.
- [stb_image.h](https://github.com/nothings/stb) - Single-file public domain (or MIT licensed) libraries for C/C++.

Most of these libraries come with the engine as a git sub-module or can be acquired by installing the Vulkan SDK.

## Installation

Glory has been set up to use the SDL and GLM libraries that come as third-party libraries with the Vulkan SDK.
Install the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) before running the premake scripts.

Edit the `vulkan_sdk` value in the `premake5.lua` file in the `GloryEngine` folder to point to where you installed the vulkan SDK.
Run the `generateprojects.bat` file under `GloryEngine/scripts` to run the premake scripts and generate the visual studio project files.
Open `GloryEngine.sln` and build the whole solution, you can run the temporary `GloryMain` project to open the editor.

## License

Personal Use Only
