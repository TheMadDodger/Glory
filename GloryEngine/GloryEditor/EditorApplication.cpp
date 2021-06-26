#include "stdafx.h"
#include "EditorApplication.h"
//#include "EditorWindow.h"

namespace Glory::Editor
{
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
			if (m_pPlatform->BeginRender()) break;
			RenderEditor();
			m_pPlatform->EndRender();
		}
	}

	void EditorApplication::RenderEditor()
	{
		//ImGui::Begin("Test");
		//
		//ImGui::End();

		m_pMainEditor->Paint();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//if (show_demo_window)
		ImGui::ShowDemoWindow();
	}
}
