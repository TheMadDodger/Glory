#include "DeleteUIElementAction.h"
#include "UIDocumentImporter.h"
#include "SetUIParentAction.h"

#include <UIDocument.h>
#include <EditorApplication.h>
#include <UIMainWindow.h>
#include <EditorResourceManager.h>

namespace Glory::Editor
{
	DeleteUIElementAction::DeleteUIElementAction(EditorApplication* pApp, UUID uuid, UIDocument* pDocument, size_t siblingIndex):
		m_ID(uuid), m_SiblingIndex(siblingIndex)
	{
		const Utils::ECS::EntityID entity = pDocument->EntityID(uuid);
		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		entities.SetMap();
		UIDocumentImporter::SerializeEntityRecursive(pApp, pDocument, entity, entities);
	}

	void DeleteUIElementAction::DeleteElement(EditorApplication* pApp, UIDocument* pDocument, Utils::YAMLFileRef& file, UUID uuid)
	{
		if (!pDocument->EntityExists(uuid)) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(uuid);
		const size_t siblingIndex = pDocument->Registry().SiblingIndex(entity);
		Undo::StartRecord("Delete UI Element", pDocument->OriginalDocumentID());
		Undo::AddAction<DeleteUIElementAction>(pApp, uuid, pDocument, siblingIndex);
		Undo::StopRecord();
		pDocument->DestroyEntity(uuid);
		SetUIParentAction::StoreDocumentState(pApp, pDocument, file["Entities"]);
	}

	void DeleteUIElementAction::OnUndo(const ActionRecord& actionRecord)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		UIMainWindow* pMainWindow = pApp->GetMainEditor().GetMainWindow<UIMainWindow>();
		UIDocument* pDocument = pMainWindow->FindEditingDocument(actionRecord.ObjectID);
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(actionRecord.ObjectID);
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::NodeValueRef rootNode = **pDocumentData;

		Utils::NodeValueRef entities = m_SerializedObject.RootNodeRef().ValueRef();
		for (auto iter = entities.Begin(); iter != entities.End(); ++iter)
		{
			Utils::NodeValueRef entity = entities[*iter];
			UIDocumentImporter::DeserializeEntity(pApp, pDocument, entity);
		}
		const Utils::ECS::EntityID entity = pDocument->EntityID(m_ID);
		pDocument->Registry().SetSiblingIndex(entity, m_SiblingIndex);
		SetUIParentAction::StoreDocumentState(pApp, pDocument, rootNode["Entities"]);
	}

	void DeleteUIElementAction::OnRedo(const ActionRecord& actionRecord)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		UIMainWindow* pMainWindow = pApp->GetMainEditor().GetMainWindow<UIMainWindow>();
		UIDocument* pDocument = pMainWindow->FindEditingDocument(actionRecord.ObjectID);
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(actionRecord.ObjectID);
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::NodeValueRef rootNode = **pDocumentData;

		pDocument->DestroyEntity(m_ID);
		SetUIParentAction::StoreDocumentState(pApp, pDocument, rootNode["Entities"]);
	}
}
