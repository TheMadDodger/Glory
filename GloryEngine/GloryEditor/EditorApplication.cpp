#include "EditorApplication.h"
#include <imgui.h>
//#include "EditorWindow.h"

namespace Glory::Editor
{
	EditorApplication* EditorApplication::m_pEditorInstance = nullptr;

	EditorApplication::EditorApplication() : m_pMainEditor(nullptr), m_pPlatform(nullptr)
	{
	}

	EditorApplication::~EditorApplication()
	{
		delete m_pMainEditor;
		m_pMainEditor = nullptr;

		delete m_pPlatform;
		m_pPlatform = nullptr;
	}

	void EditorApplication::Destroy()
	{
		m_pMainEditor->Destroy();
		m_pPlatform->Destroy();
	}

	void EditorApplication::Run()
	{
		while (true)
		{
			if (m_pPlatform->PollEvents()) break;
			m_pPlatform->BeginRender();
			m_pPlatform->WaitIdle();
			RenderEditor();
			m_pPlatform->EndRender();
			m_pPlatform->WaitIdle();
		}
	}

	EditorPlatform* EditorApplication::GetEditorPlatform()
	{
		return m_pPlatform;
	}

	MainEditor* EditorApplication::GetMainEditor()
	{
		return m_pMainEditor;
	}

	EditorApplication* EditorApplication::GetInstance()
	{
		return m_pEditorInstance;
	}

	void EditorApplication::RenderEditor()
	{
		m_pMainEditor->Paint();

		ImGui::ShowDemoWindow();
	}
}
