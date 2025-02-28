#include "UIEditorExtension.h"
#include "UIEditor.h"
#include "UIDocumentImporter.h"
#include "UIMainWindow.h"

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

#include <IconsFontAwesome6.h>

EXTENSION_CPP(UIEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_UIEditor = "Open UI Editor";

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
			UIEntity entity = pUIDocument->Create<UIText>();
			entity.GetComponent<UIText>().m_Text = "Hello World!";
			entity.GetComponent<UITransform>().m_Rect = glm::vec4(200.0f, 400.0f, 0.0f, 0.0f);
			UIEntity imageEntity = pUIDocument->Create<UIImage>();
			//imageEntity.GetComponent<UIImage>().m_Image = "Hello World!";
			imageEntity.GetComponent<UITransform>().m_Rect = glm::vec4(400.0f, 600.0f, 0.0f, 0.0f);

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

		pEngine->GetSceneManager()->ComponentTypesInstance();

		MenuBar::AddMenuItem("Window/UI Editor", [&editor]() { editor.GetWindow<UIMainWindow, UIEditor>(); }, NULL, Shortcut_Window_UIEditor);

		ObjectMenu::AddMenuItem("Create/UI Document", OnCreateUIDocument, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);

		EntitySceneObjectEditor::AddComponentIcon<UIRenderer>(ICON_FA_LIST);
		OBJECT_CREATE_MENU(UIRenderer, UIRenderer);

		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();
		Utils::ECS::ComponentTypes::SetInstance(pModule->GetComponentTypes());
	}
}
