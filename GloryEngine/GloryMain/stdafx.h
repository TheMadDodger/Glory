#pragma once
// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <fstream>
#include <GloryCore.h>
#include <SDLWindowModule.h>
#include <VulkanGraphicsModule.h>
#include <SDLImageLoaderModule.h>
#include <SDL2/SDL_video.h>
#include <ASSIMPModule.h>
#include <FileLoaderModule.h>
#include <ShaderLoaderModule.h>
#include <GLSLShaderLoader.h>
#include <EntitySceneScenesModule.h>
#include <OpenGLGraphicsModule.h>
#include <GL/glew.h>

#pragma comment (lib,"opengl32.lib")