#pragma once
#include <vector>
#include <imgui_internal.h>
#include "Editor.h"
#include "GloryContext.h"

typedef void(__cdecl* SetContextProc)(Glory::GloryContext*, ImGuiContext* pImGUIContext);

#define EXTENSION_H extern "C" GLORY_EDITOR_EXTENSION_API Glory::Editor::BaseEditorExtension* LoadExtension(); \
extern "C" GLORY_EDITOR_EXTENSION_API void SetContext(Glory::GloryContext * pContext, ImGuiContext * pImGUIContext);

#define EXTENSION_CPP(ext) Glory::Editor::BaseEditorExtension* LoadExtension() \
{ \
	return new Glory::Editor::ext(); \
} \
\
void SetContext(Glory::GloryContext* pContext, ImGuiContext* pImGUIContext) \
{ \
	Glory::GloryContext::SetContext(pContext); \
	ImGui::SetCurrentContext(pImGUIContext); \
}

namespace Glory::Editor
{
	class BaseEditorExtension
	{
	public:
		GLORY_EDITOR_API BaseEditorExtension();
		virtual GLORY_EDITOR_API ~BaseEditorExtension();

		GLORY_EDITOR_API void SetSetContextProc(SetContextProc proc);

	protected:
		virtual void Initialize() = 0;

	private:
		void SetCurrentContext();

	private:
		friend class EditorApplication;
		SetContextProc m_SetContextProc;
	};
}
