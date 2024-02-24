#include "Configuration.h"

#include <crtdbg.h>
#include <Windows.h>

typedef void(__cdecl* CreateAppProc)(const char*);
typedef void(__cdecl* InitializeAppProc)(int, char*[]);
typedef void(__cdecl* RunAppProc)();
typedef void(__cdecl* CleanupAppProc)();
typedef void(__cdecl* LoadSceneProc)(const char*);

HMODULE RuntimeAppLib;

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    {
		RuntimeAppLib = LoadLibrary(L"GloryRuntimeApplication.dll");
		if (RuntimeAppLib == NULL)
			return -1;

		CreateAppProc create = (CreateAppProc)GetProcAddress(RuntimeAppLib, "CreateApp");
		InitializeAppProc init = (InitializeAppProc)GetProcAddress(RuntimeAppLib, "InitializeApp");
		RunAppProc run = (RunAppProc)GetProcAddress(RuntimeAppLib, "RunApp");
		CleanupAppProc cleanup = (CleanupAppProc)GetProcAddress(RuntimeAppLib, "CleanupApp");
		LoadSceneProc loadScene = (LoadSceneProc)GetProcAddress(RuntimeAppLib, "LoadScene");

		create(Config::AppName);
		init(argc, argv);
		loadScene(Config::EntryScene);
		run();
		cleanup();

		FreeLibrary(RuntimeAppLib);
    }

    _CrtDumpMemoryLeaks();

    return 0;
}