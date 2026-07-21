#include "TestEngineWindow.h"
#include "TestEngineExtension.h"

#include <imgui_te_ui.h>

namespace Glory::Editor
{
	TestEngineWindow::TestEngineWindow(): EditorWindowTemplate("Test Engine", 600.0f, 600.0f)
	{
	}

	void TestEngineWindow::DrawWindow()
	{
		ImGuiTestEngine_ShowTestEngineWindows(TestEngineEditorExtension::GetTestEngine(), &m_IsOpen);
		if (!m_IsOpen)
			Close();
	}
}
