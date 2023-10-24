#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"
#include "PhysicsBodyEditor.h"
#include "ScriptedComponentEditor.h"
#include "CharacterControllerEditor.h"
#include "CreateEntityObjectsCallbacks.h"

#include <SceneManager.h>
#include <EditorPlayer.h>
#include <Components.h>
#include <Reflection.h>
#include <PrefabData.h>
#include <EditorAssetDatabase.h>
#include <FileBrowser.h>
#include <EditorSceneManager.h>
#include <ObjectMenu.h>
#include <SceneGraphWindow.h>
#include <Dispatcher.h>

#define OBJECT_CREATE_MENU(name, component) std::stringstream name##MenuName; \
name##MenuName << STRINGIFY(Create/Entity Object/) << EntitySceneObjectEditor::GetComponentIcon<component>() << "  " << STRINGIFY(name); \
ObjectMenu::AddMenuItem(name##MenuName.str(), Create##name, T_SceneObject | T_Scene | T_Hierarchy);

namespace Glory::Editor
{
	EntityScenesEditorExtension::EntityScenesEditorExtension()
	{
	}

	EntityScenesEditorExtension::~EntityScenesEditorExtension()
	{
	}

	void EntityScenesEditorExtension::Initialize()
	{
		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();
		Editor::RegisterEditor<TransformEditor>();
		Editor::RegisterEditor<ScriptedComponentEditor>();
		Editor::RegisterEditor<PhysicsBodyEditor>();
		Editor::RegisterEditor<CharacterControllerEditor>();

		PropertyDrawer::RegisterPropertyDrawer<SimplePropertyDrawerTemplate<MeshMaterial>>();

		SceneGraphWindow::SetDrawObjectNameCallback(EntitySceneObjectEditor::DrawObjectNodeName);
		SceneGraphWindow::SetSearchCompareCallback(EntitySceneObjectEditor::SearchCompare);
		SceneGraphWindow::SetSearchTooltipCallback([]() {
			ImGui::SetTooltip("Search by object name,\nyou can also search by component by typing c:componenttype");
		});

		OBJECT_CREATE_MENU(Mesh, MeshRenderer);
		OBJECT_CREATE_MENU(Model, ModelRenderer);
		OBJECT_CREATE_MENU(Camera, CameraComponent);
		OBJECT_CREATE_MENU(Light, LightComponent);
		OBJECT_CREATE_MENU(Scripted, ScriptedComponent);
		OBJECT_CREATE_MENU(PhysicsBody, PhysicsBody);
		OBJECT_CREATE_MENU(Character, CharacterController);

		ObjectMenu::AddMenuItem("Convert to Prefab", &ConvertToPrefabMenuItem, T_SceneObject);
		ObjectMenu::AddMenuItem("Unpack Prefab", &UnpackPrefabMenuItem, T_SceneObject);

		FileBrowserItem::ObjectDNDEventDispatcher().AddListener([](const FileBrowserItem::ObjectDNDEvent& e) {
			ConvertToPrefab(e.Object, e.Path);
		});
	}

	void EntityScenesEditorExtension::ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&)
	{
		SceneObject* pSceneObject = (SceneObject*)pObject;
		const std::filesystem::path path = FileBrowser::GetCurrentPath();
		ConvertToPrefab(pSceneObject, path);
	}

	void EntityScenesEditorExtension::ConvertToPrefab(SceneObject* pObject, std::filesystem::path path)
	{
		PrefabData* pPrefab = PrefabData::CreateFromSceneObject(pObject);
		path.append(pObject->Name() + ".gentity");
		const UUID prefabUUID = EditorAssetDatabase::CreateAsset(pPrefab, path.string());
		GScene* pScene = pObject->GetScene();
		pScene->SetPrefab(pObject, prefabUUID);
		EditorSceneManager::SetSceneDirty(pScene);
	}

	void EntityScenesEditorExtension::UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&)
	{
		SceneObject* pSceneObject = (SceneObject*)pObject;
		GScene* pScene = pSceneObject->GetScene();
		if (!pScene->Prefab(pSceneObject->GetUUID())) return;
		pScene->UnsetPrefab(pSceneObject);
		EditorSceneManager::SetSceneDirty(pScene);
	}
}

EXTENSION_CPP(EntityScenesEditorExtension)
