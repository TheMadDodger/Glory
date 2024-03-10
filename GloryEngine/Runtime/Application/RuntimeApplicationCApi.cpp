#include "pch.h"
#include "RuntimeApplicationCApi.h"
#include "GloryRuntimeApplication.h"

#include <GloryRuntime.h>
#include <Console.h>

Glory::GloryRuntimeApplication runtimeApp;

void CreateApp(const char* name)
{
	runtimeApp.SetName(name);
}

void InitializeApp(int argc, char* argv[])
{
	runtimeApp.Initialize(argc, argv);
}

void RunApp()
{
	runtimeApp.Run();
}

void CleanupApp()
{
	runtimeApp.Cleanup();
}

void LoadScene(const uint64_t uuid)
{
	runtimeApp.Runtime().LoadScene(uuid);
}

void RunCommand(const char* command)
{
	runtimeApp.Runtime().GetEngine()->GetConsole().ExecuteCommand(command);
}
