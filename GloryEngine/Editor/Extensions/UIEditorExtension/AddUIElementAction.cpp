#include "AddUIElementAction.h"

#include <UIDocumentData.h>
#include <UIDocument.h>

#include <EditorApplication.h>
#include <UIMainWindow.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <SetUIParentAction.h>

/* TEMP */
#include <UIComponents.h>

namespace Glory::Editor
{
	AddUIElementAction::AddUIElementAction(uint32_t type, UUID uuid, UUID parent, size_t siblingIndex):
		m_Type(type), m_ID(uuid), m_Parent(parent), m_SiblingIndex(siblingIndex)
	{
	}

	UUID AddUIElementAction::AddElement(Engine* pEngine, UIDocument* pDocument, Utils::YAMLFileRef& file,
		std::string_view name, uint32_t type, UUID parent, size_t siblingIndex)
	{
		const UUID newEntityID{};
		const Utils::ECS::EntityID parentEntity = parent ? pDocument->EntityID(parent) : 0;
		const Utils::ECS::EntityID entity = pDocument->CreateEntity(name, newEntityID);
		pDocument->Registry().CreateComponent(entity, type, UUID());
		pDocument->Registry().AddComponent<UIInteraction>(entity, UUID());
		pDocument->Registry().SetParent(entity, parentEntity);
		pDocument->Registry().SetSiblingIndex(entity, siblingIndex);

		Undo::StartRecord("Add UI Element", pDocument->OriginalDocumentID());
		Undo::AddAction<AddUIElementAction>(type, newEntityID, parent, siblingIndex);
		Undo::StopRecord();

		SetUIParentAction::StoreDocumentState(pEngine, pDocument, file["Entities"]);
		pDocument->SetDrawDirty();
		return newEntityID;
	}

	void AddUIElementAction::OnUndo(const ActionRecord& actionRecord)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		UIMainWindow* pMainWindow = pApp->GetMainEditor().GetMainWindow<UIMainWindow>();
		UIDocument* pDocument = pMainWindow->FindEditingDocument(actionRecord.ObjectID);
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(actionRecord.ObjectID);
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocumentData;

		pDocument->DestroyEntity(m_ID);
		SetUIParentAction::StoreDocumentState(pEngine, pDocument, file["Entities"]);
	}

	void AddUIElementAction::OnRedo(const ActionRecord& actionRecord)
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
		const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(m_Type);
		const Utils::ECS::EntityID entity = pDocument->CreateEntity(pType->TypeName(), m_ID);
		const Utils::ECS::EntityID parent = m_Parent ? pDocument->EntityID(m_Parent) : 0;
		registry.CreateComponent(entity, m_Type, UUID());
		registry.AddComponent<UIInteraction>(entity, UUID());
		registry.SetParent(entity, parent);
		registry.SetSiblingIndex(entity, m_SiblingIndex);
		SetUIParentAction::StoreDocumentState(pEngine, pDocument, file["Entities"]);
	}
}
