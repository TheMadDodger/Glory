#include "DefaultComponentEditor.h"

#include <imgui.h>
#include <PropertyDrawer.h>
#include <Undo.h>
#include <TypeData.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	DefaultComponentEditor::DefaultComponentEditor()
	{
	}

	DefaultComponentEditor::~DefaultComponentEditor()
	{
	}

	bool DefaultComponentEditor::OnGUI()
	{
		Undo::StartRecord("Property Change", m_pTarget->GetUUID(), true);
		bool change = false;
		EntityComponentObject* pComponentObject = (EntityComponentObject*)GetTarget();
		const uint32_t hash = pComponentObject->ComponentType();
		Utils::ECS::EntityRegistry* pRegistry = pComponentObject->GetRegistry();
		const Utils::ECS::EntityID entity = pComponentObject->EntityID();
		const UUID componentID = pComponentObject->GetUUID();

		const TypeData* pTypeData = Reflect::GetTyeData(hash);

		Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(hash);
		bool active = pTypeView->IsActive(entity);
		if (EditorUI::CheckBox("Active", &active))
		{
			pTypeView->SetActive(entity, active);
			change = true;
		}

		void* pAddress = pRegistry->GetComponentAddress(entity, componentID);

		if (pTypeData)
		{
			change |= PropertyDrawer::DrawProperty("", pTypeData, pAddress, 0);
		}

		Undo::StopRecord();

		if (change)
			pRegistry->GetTypeView(hash)->Invoke(Utils::ECS::InvocationType::OnValidate, pRegistry, entity, pAddress);

		return change;
	}

	void DefaultComponentEditor::Initialize()
	{
		EntityComponentObject* pEntityComponentObject = (EntityComponentObject*)m_pTarget;
		const TypeData* pTypeData = Reflect::GetTyeData(pEntityComponentObject->ComponentType());
		m_Name = pTypeData->TypeName();
	}

	std::string DefaultComponentEditor::Name()
	{
		return m_Name;
	}
}
