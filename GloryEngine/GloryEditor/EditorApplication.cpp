#include "EditorApplication.h"
#include <imgui.h>
#include <Console.h>

namespace Glory::Editor
{
	EditorApplication* EditorApplication::m_pEditorInstance = nullptr;

	EditorApplication::EditorApplication(const EditorCreateInfo& createInfo) : m_pMainEditor(nullptr), m_pPlatform(nullptr)
	{
		// Copy the optional modules into the optional modules vector
		if (createInfo.ExtensionsCount > 0 && createInfo.pExtensions != nullptr)
		{
			m_pExtensions.resize(createInfo.ExtensionsCount);
			for (size_t i = 0; i < createInfo.ExtensionsCount; i++)
			{
				m_pExtensions[i] = createInfo.pExtensions[i];
			}
		}
	}

	EditorApplication::~EditorApplication()
	{
		delete m_pMainEditor;
		m_pMainEditor = nullptr;

		delete m_pPlatform;
		m_pPlatform = nullptr;
	}

	void EditorApplication::InitializeExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->RegisterEditors();
		}
	}

	void EditorApplication::Destroy()
	{
		m_pMainEditor->Destroy();
		m_pPlatform->Destroy();
	}

	void EditorApplication::Run(Game& game)
	{
		game.GetEngine()->StartThreads();
		while (true)
		{
			Console::Update();
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
		m_pMainEditor->PaintEditor();

		ImGui::ShowDemoWindow();
	}
}
