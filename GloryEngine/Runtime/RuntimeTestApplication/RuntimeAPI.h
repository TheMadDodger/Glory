#pragma once

typedef void(__cdecl* CreateAppProc)(const char*);
typedef void(__cdecl* InitializeAppProc)(int, char* []);
typedef void(__cdecl* RunAppProc)();
typedef void(__cdecl* CleanupAppProc)();
typedef void(__cdecl* LoadSceneProc)(const char*);
typedef void(__cdecl* RunCommandProc)(const char*);

CreateAppProc create;
InitializeAppProc init;
RunAppProc run;
CleanupAppProc cleanup;
LoadSceneProc loadScene;
RunCommandProc runCommand;
