#include "UIToolsExtension.h"
#include "UIEditor.h"
#include "FontImporter.h"

#include <UIComponents.h>

#include <SceneManager.h>

#include <MenuBar.h>
#include <EntitySceneObjectEditor.h>
#include <ObjectMenu.h>
#include <CreateObjectAction.h>
#include <CreateEntityObjectsCallbacks.h>
#include <EditableEntity.h>
#include <EntityEditor.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(UIToolsExtension)

#define OBJECT_CREATE_MENU(name, component) std::stringstream name##MenuName; \
name##MenuName << STRINGIFY(Create/Entity Object/) << EntitySceneObjectEditor::GetComponentIcon<component>() << "  " << STRINGIFY(name); \
ObjectMenu::AddMenuItem(name##MenuName.str(), Create##name, T_SceneObject | T_Scene | T_Hierarchy);

namespace Glory::Editor
{
	CREATE_OBJECT_CALLBACK_CPP(TextComponent, TextComponent, ());

	static constexpr char* Shortcut_Window_UIEditor = "Open UI Editor";

	FontImporter Font_Importer;

	UIToolsExtension::UIToolsExtension()
	{
	}

	UIToolsExtension::~UIToolsExtension()
	{
	}

	void UIToolsExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());

		pEngine->GetSceneManager()->ComponentTypesInstance();

		MenuBar::AddMenuItem("Window/UI Editor", []() { EditorWindow::GetWindow<UIEditor>(); }, NULL, Shortcut_Window_UIEditor);

		static constexpr char* textIcon = ICON_FA_FONT;
		EntitySceneObjectEditor::AddComponentIcon<TextComponent>(textIcon);

		OBJECT_CREATE_MENU(TextComponent, TextComponent);

		Importer::Register(&Font_Importer);
	}
}
