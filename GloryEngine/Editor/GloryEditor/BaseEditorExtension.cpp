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
		ImGuiContext* pCurrentEditorContext = ImGui::GetCurrentContext();
		m_SetContextProc(pCurrentEditorContext);
	}
}
