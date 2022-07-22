#include "EditorContext.h"

namespace Glory::Editor
{
	EditorContext* EditorContext::m_pContext = nullptr;

	EditorContext* EditorContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new EditorContext();
		return m_pContext;
	}

	void EditorContext::DestroyContext()
	{
		if (!m_pContext) return;
		delete m_pContext;
		m_pContext = nullptr;
	}

	void EditorContext::SetContext(EditorContext* pContext)
	{
		m_pContext = pContext;
	}

	EditorContext* EditorContext::GetContext()
	{
		if (ImGui::GetCurrentContext() == nullptr) ImGui::SetCurrentContext(m_pContext->m_pIMGUIContext);
		return m_pContext;
	}

	void EditorContext::Initialize()
	{
		m_pIMGUIContext = ImGui::GetCurrentContext();
	}

	Editors* EditorContext::GetEditors()
	{
		return GetContext()->m_Editors;
	}

	EditorContext::EditorContext() : m_Editors(new Editors), m_pIMGUIContext(nullptr)
	{
	}

	EditorContext::~EditorContext()
	{
		delete m_Editors;
		m_Editors = nullptr;
	}

	Editors::Editors()
	{
	}

	Editors::~Editors()
	{
	}
}
