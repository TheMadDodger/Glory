#include "BaseEditorExtension.h"

namespace Glory::Editor
{
	BaseEditorExtension::BaseEditorExtension() : m_SetContextProc(NULL)
	{
	}

	BaseEditorExtension::~BaseEditorExtension()
	{
	}

	void BaseEditorExtension::SetSetContextProc(SetContextProc proc)
	{
		m_SetContextProc = proc;
	}

	void BaseEditorExtension::SetCurrentContext()
	{
		if (m_SetContextProc == NULL) return;
		GloryContext* pCurrentContext = GloryContext::GetContext();
		ImGuiContext* pCurrentEditorContext = ImGui::GetCurrentContext();
		m_SetContextProc(pCurrentContext, pCurrentEditorContext);
	}
}
