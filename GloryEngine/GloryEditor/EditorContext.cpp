#include "EditorContext.h"
#include "Gizmos.h"

namespace Glory::Editor
{
	EditorContext* EditorContext::m_pContext = nullptr;

	GLORY_EDITOR_API EditorContext* EditorContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new EditorContext();
		return m_pContext;
	}

	GLORY_EDITOR_API Gizmos* EditorContext::GetGizmos()
	{
		return GetContext()->m_pGizmos;
	}

	void EditorContext::DestroyContext()
	{
		if (!m_pContext) return;
		delete m_pContext;
		m_pContext = nullptr;
	}

	GLORY_EDITOR_API void EditorContext::SetContext(EditorContext* pContext)
	{
		m_pContext = pContext;
	}

	GLORY_EDITOR_API EditorContext* EditorContext::GetContext()
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

	EditorContext::EditorContext() : m_Editors(new Editors), m_pIMGUIContext(nullptr), m_pGizmos(new Gizmos())
	{
	}

	EditorContext::~EditorContext()
	{
		delete m_Editors;
		m_Editors = nullptr;

		delete m_pGizmos;
		m_pGizmos = nullptr;
	}

	Editors::Editors()
	{
	}

	Editors::~Editors()
	{
	}
}
