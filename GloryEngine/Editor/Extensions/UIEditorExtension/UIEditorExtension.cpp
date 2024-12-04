#include "UIEditorExtension.h"
#include "UIEditor.h"

#include <SceneManager.h>

#include <MenuBar.h>

EXTENSION_CPP(UIEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_UIEditor = "Open UI Editor";

	UIEditorExtension::UIEditorExtension()
	{
	}

	UIEditorExtension::~UIEditorExtension()
	{
	}

	void UIEditorExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());

		pEngine->GetSceneManager()->ComponentTypesInstance();

		MenuBar::AddMenuItem("Window/UI Editor", []() { EditorWindow::GetWindow<UIEditor>(); }, NULL, Shortcut_Window_UIEditor);
	}
}
