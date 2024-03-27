#include "Configuration.h"

#include <crtdbg.h>
#include <Windows.h>

HMODULE RuntimeAppLib;

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    {
		RuntimeAppLib = LoadLibrary(L"GloryRuntimeApplication.dll");
		if (RuntimeAppLib == NULL)
			return -1;

		create = (CreateAppProc)GetProcAddress(RuntimeAppLib, "CreateApp");
		init = (InitializeAppProc)GetProcAddress(RuntimeAppLib, "InitializeApp");
		run = (RunAppProc)GetProcAddress(RuntimeAppLib, "RunApp");
		cleanup = (CleanupAppProc)GetProcAddress(RuntimeAppLib, "CleanupApp");
		loadScene = (LoadSceneProc)GetProcAddress(RuntimeAppLib, "LoadScene");
		runCommand = (RunCommandProc)GetProcAddress(RuntimeAppLib, "RunCommand");

		create(Config::AppName);
		init(argc, argv);
		Exec();
		run();
		cleanup();

		FreeLibrary(RuntimeAppLib);
    }

    _CrtDumpMemoryLeaks();

    return 0;
}