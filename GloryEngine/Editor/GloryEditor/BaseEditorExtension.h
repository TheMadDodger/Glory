#pragma once
#include "Editor.h"

#include <vector>
#include <imgui_internal.h>

typedef void(__cdecl* SetContextProc)(ImGuiContext* pImGUIContext);

#define EXTENSION_H extern "C" GLORY_EDITOR_EXTENSION_API Glory::Editor::BaseEditorExtension* LoadExtension(); \
extern "C" GLORY_EDITOR_EXTENSION_API void SetContext(ImGuiContext * pImGUIContext);

#define EXTENSION_CPP(ext) Glory::Editor::BaseEditorExtension* LoadExtension() \
{ \
	return new Glory::Editor::ext(); \
} \
\
void SetContext(ImGuiContext* pImGUIContext) \
{ \
	ImGui::SetCurrentContext(pImGUIContext); \
}

namespace std::filesystem
{
	class path;
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
		virtual void Update() {};
		virtual void OnBeginPackage(const std::filesystem::path& path) {};
		virtual void OnGenerateConfigExec(std::ofstream& stream) {};
		virtual void OnEndPackage(const std::filesystem::path& path) {};

	private:
		void SetCurrentContext();

	private:
		friend class EditorApplication;
		SetContextProc m_SetContextProc;
	};
}
