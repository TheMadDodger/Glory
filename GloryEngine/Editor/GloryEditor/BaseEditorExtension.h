#pragma once
#include "Editor.h"

#include <vector>
#include <imgui_internal.h>

typedef void(__cdecl* SetContextProc)(ImGuiContext* pImGUIContext);

#define EXTENSION_H(api) extern "C" api Glory::Editor::BaseEditorExtension* LoadExtension(); \
extern "C" api void SetContext(ImGuiContext* pImGUIContext);

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
		GLORY_EDITOR_API void BroadcastMessage(std::string_view message, void* data);

	protected:
		virtual void Initialize() = 0;
		virtual void Update() {};
		virtual void OnBeginPackage(const std::filesystem::path& path) {};
		virtual void OnGenerateConfigExec(std::ofstream& stream) {};
		virtual void OnEndPackage(const std::filesystem::path& path) {};
		virtual void OnBroadcastMessage(std::string_view message, void* data) {};

	private:
		void SetCurrentContext();

	private:
		friend class EditorApplication;
		SetContextProc m_SetContextProc;
	};
}
