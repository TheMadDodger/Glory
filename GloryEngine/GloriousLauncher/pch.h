#pragma once
#define SDL_MAIN_HANDLED

#include <crtdbg.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <functional>

#define _CRTDBG_MAP_ALLOC
#define NOMINMAX

#pragma comment (lib,"opengl32.lib")
#pragma comment (lib,"SDL2.lib")