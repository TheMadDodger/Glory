#pragma once
#include "Visibility.h"

extern "C" GLORY_RUNTIME_APPLICATION_API void CreateApp(const char* name);
extern "C" GLORY_RUNTIME_APPLICATION_API void InitializeApp(int argc, char* argv[]);
extern "C" GLORY_RUNTIME_APPLICATION_API void RunApp();
extern "C" GLORY_RUNTIME_APPLICATION_API void CleanupApp();
extern "C" GLORY_RUNTIME_APPLICATION_API void LoadScene(const char* path);
