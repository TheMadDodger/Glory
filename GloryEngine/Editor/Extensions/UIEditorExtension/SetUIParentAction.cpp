#include "SetUIParentAction.h"
#include "UIMainWindow.h"
#include "UIDocumentImporter.h"

#include <UIDocument.h>

#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>

#include <NodeRef.h>

namespace Glory::Editor
{
	SetUIParentAction::SetUIParentAction(UUID toReParent, UUID oldParent, UUID newParent, size_t siblingIndex, size_t newSiblingIndex):
		m_ToReparent(toReParent), m_OldParent(oldParent),
		m_NewParent(newParent), m_OldSiblingIndex(siblingIndex),
		m_NewSiblingIndex(newSiblingIndex)
	{
	}

	void SetUIParentAction::StoreDocumentState(Engine* pEngine, UIDocument* pDocument, Utils::NodeValueRef entities)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->Registry();
		entities.SetMap();
		for (size_t i = 0; i < registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(0, i);
			UIDocumentImporter::SerializeEntityRecursive(pEngine, pDocument, child, entities);
		}
		EditorAssetDatabase::SetAssetDirty(pDocument->OriginalDocumentID());
	}

	void SetUIParentAction::OnUndo(const ActionRecord& actionRecord)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		UIMainWindow* pMainWindow = pApp->GetMainEditor().GetMainWindow<UIMainWindow>();
		UIDocument* pDocument = pMainWindow->FindEditingDocument(actionRecord.ObjectID);
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(actionRecord.ObjectID);
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocumentData;

		Utils::ECS::EntityRegistry& registry = pDocument->Registry();
		const Utils::ECS::EntityID toReParent = pDocument->EntityID(m_ToReparent);
		const Utils::ECS::EntityID parent = m_OldParent ? pDocument->EntityID(m_OldParent) : 0;
		registry.SetParent(toReParent, parent);
		registry.SetSiblingIndex(toReParent, m_OldSiblingIndex);

		StoreDocumentState(pEngine, pDocument, file["Entities"]);
	}

	void SetUIParentAction::OnRedo(const ActionRecord& actionRecord)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		UIMainWindow* pMainWindow = pApp->GetMainEditor().GetMainWindow<UIMainWindow>();
		UIDocument* pDocument = pMainWindow->FindEditingDocument(actionRecord.ObjectID);
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(actionRecord.ObjectID);
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocumentData;

		Utils::ECS::EntityRegistry& registry = pDocument->Registry();
		const Utils::ECS::EntityID toReParent = pDocument->EntityID(m_ToReparent);
		const Utils::ECS::EntityID parent = m_NewParent ? pDocument->EntityID(m_NewParent) : 0;
		registry.SetParent(toReParent, parent);
		registry.SetSiblingIndex(toReParent, m_NewSiblingIndex);

		StoreDocumentState(pEngine, pDocument, file["Entities"]);
	}
}
