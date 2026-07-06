#include "TestEngineExtension.h"

EXTENSION_CPP(TestEngineEditorExtension)

namespace Glory::Editor
{
	static constexpr const char* Shortcut_Window_TestEngine = "Open Test Engine";

	TestEngineEditorExtension::TestEngineEditorExtension(): m_pEngine(nullptr)
	{
	}

	TestEngineEditorExtension::~TestEngineEditorExtension()
	{
		// May block until TestFunc thread/coroutine joins
		ImGuiTestEngine_Stop(m_pEngine);

		// We shutdown the Dear ImGui context _before_ the test engine context, so .ini data may be saved.
		//ImGui::DestroyContext();
		ImGuiTestEngine_DestroyContext(m_pEngine);
	}

	void TestEngineEditorExtension::Initialize()
	{
			// Initialize Test Engine
		m_pEngine = ImGuiTestEngine_CreateContext();
		ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(m_pEngine);
		test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
		test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;

		// Register your Tests
		//RegisterMyTests(engine); // will call IM_REGISTER_TEST() etc.

		// Start test engine
		ImGuiTestEngine_Start(m_pEngine, ImGui::GetCurrentContext());

		// Optional: use default crash handler. You may use your own crash handler and call ImGuiTestEngine_CrashHandler() from it.
		ImGuiTestEngine_InstallDefaultCrashHandler();
	}
}
