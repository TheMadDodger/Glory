#include "pch.h"
#include "HubWindow.h"
#include "ImGuiImpl.h"
#include "LauncherHub.h"
#include "EditorManager.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::EditorLauncher::EditorManager::GetInstalledEditors();

        Glory::EditorLauncher::HubWindow window("Glorious Hub");
        window.Initialize();

        GLenum result = glewInit();
        if (result != GLEW_OK)
        {
            fprintf(stderr, "Failed to initialize GLEW!\n");
            return 1;
        }

        Glory::EditorLauncher::ImGuiImpl imguiImpl(&window);
        imguiImpl.Initialize();

        Glory::EditorLauncher::LauncherHub launcherHub(&imguiImpl);
        launcherHub.Run();
    }

    _CrtDumpMemoryLeaks();

	return 0;
}