#include "UIEditorExtension.h"
#include "UIEditor.h"
#include "UIDocumentImporter.h"
#include "UIDocumentThumbnailGenerator.h"
#include "UIMainWindow.h"
#include "UIElementInspector.h"
#include "UIElementsGraphWindow.h"
#include "AddUIElementWindow.h"
#include "ConstraintDrawer.h"

#include <SceneManager.h>
#include <UIDocumentData.h>
#include <UIRendererModule.h>

#include <UIComponents.h>

#include <MenuBar.h>
#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <EditorAssetCallbacks.h>
#include <ObjectMenuCallbacks.h>
#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <CreateObjectAction.h>
#include <EntityEditor.h>
#include <ThumbnailManager.h>

#include <IconsFontAwesome6.h>
#include <PropertyDrawer.h>

EXTENSION_CPP(UIEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_UIEditor = "Open UI Preview";
	static constexpr char* Shortcut_Window_UIGraph = "Open UI Graph";
	static constexpr char* Shortcut_Window_UIInspector = "Open UI Inspector";
	static constexpr char* Shortcut_Window_UILibrary = "Open UI Library";

	UIDocumentImporter importer;
	UIMainWindow UIEditorMainWindow;

	void CreateUIRenderer(Object* pObject, const ObjectMenuType& currentMenu)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		pEngine->GetSceneManager()->ComponentTypesInstance();

		Entity newEntity = CreateNewEmptyObject(pObject, "UIRenderer", currentMenu);
		Undo::StartRecord("Create Empty Object", newEntity.EntityUUID());
		newEntity.AddComponent<UIRenderer>();
		Undo::AddAction<CreateObjectAction>(newEntity.GetScene());
		Selection::SetActiveObject(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
		Undo::StopRecord();

		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();
		Utils::ECS::ComponentTypes::SetInstance(pModule->GetComponentTypes());
	}

	UIEditorExtension::UIEditorExtension()
	{
	}

	UIEditorExtension::~UIEditorExtension()
	{
	}

	void OnCreateUIDocument(Object*, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New UI Document.gui"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gui");
			if (std::filesystem::exists(finalPath)) return;
			UIDocumentData* pUIDocument = new UIDocumentData();
			EditorAssetDatabase::CreateAsset(pUIDocument, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void UIEditorExtension::Initialize()
	{
		Importer::Register(&importer);

		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();
		editor.RegisterMainWindow(&UIEditorMainWindow);
		Engine* pEngine = pApp->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());
		Reflect::RegisterType<UIElementType>();

		pEngine->GetSceneManager()->ComponentTypesInstance();

		MenuBar::AddMenuItem("Window/UI Editor/Preview", [&editor]() { editor.GetWindow<UIMainWindow, UIEditor>(); }, NULL, Shortcut_Window_UIEditor);
		MenuBar::AddMenuItem("Window/UI Editor/Inspector", [&editor]() { editor.GetWindow<UIMainWindow, UIElementInspector>(); }, NULL, Shortcut_Window_UIInspector);
		MenuBar::AddMenuItem("Window/UI Editor/Graph", [&editor]() { editor.GetWindow<UIMainWindow, UIElementsGraphWindow>(); }, NULL, Shortcut_Window_UIGraph);
		MenuBar::AddMenuItem("Window/UI Editor/Library", [&editor]() { editor.GetWindow<UIMainWindow, AddUIElementWindow>(); }, NULL, Shortcut_Window_UILibrary);
		ThumbnailManager::AddGenerator<UIDocumentThumbnailGenerator>();

		UIMainWindow* pMainWindow = editor.FindMainWindow<UIMainWindow>();
		pMainWindow->GetWindow<UIEditor>();
		pMainWindow->GetWindow<UIElementInspector>();
		pMainWindow->GetWindow<UIElementsGraphWindow>();
		pMainWindow->GetWindow<AddUIElementWindow>();

		ObjectMenu::AddMenuItem("Create/UI Document", OnCreateUIDocument, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);

		EntitySceneObjectEditor::AddComponentIcon<UIRenderer>(ICON_FA_LIST);
		OBJECT_CREATE_MENU(UIRenderer, UIRenderer);

		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();
		Utils::ECS::ComponentTypes::SetInstance(pModule->GetComponentTypes());

		PropertyDrawer::RegisterPropertyDrawer<ConstraintDrawer<XConstraint>>();
		PropertyDrawer::RegisterPropertyDrawer<ConstraintDrawer<YConstraint>>();
		PropertyDrawer::RegisterPropertyDrawer<ConstraintDrawer<WidthConstraint>>();
		PropertyDrawer::RegisterPropertyDrawer<ConstraintDrawer<HeightConstraint>>();
	}
}
