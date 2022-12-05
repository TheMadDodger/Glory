#pragma once
#include <vector>
#include <imgui_internal.h>
#include "Editor.h"
#include "GloryContext.h"

typedef void(__cdecl* SetContextProc)(Glory::GloryContext*, ImGuiContext* pImGUIContext);

namespace Glory::Editor
{
	class BaseEditorExtension
	{
	public:
		GLORY_EDITOR_API BaseEditorExtension();
		virtual GLORY_EDITOR_API ~BaseEditorExtension();

		GLORY_EDITOR_API void SetSetContextProc(SetContextProc proc);

	protected:
		virtual void RegisterEditors() = 0;

	private:
		void SetCurrentContext();

	private:
		friend class EditorApplication;
		SetContextProc m_SetContextProc;
	};
}
