#pragma once
#include "Editor.h"

#include <Object.h>
#include <EntityComponentObject.h>
#include <PropertyDrawer.h>
#include <TypeData.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	template<typename TEditor, typename TComponent>
	class EntityComponentEditor : public EditorTemplate<TEditor, EntityComponentObject>
	{
	public:
		EntityComponentEditor() : EditorTemplate(ResourceType::GetHash<TComponent>()), m_pComponentObject(nullptr) {}
		virtual ~EntityComponentEditor() {}

	protected:
		TComponent& GetTargetComponent()
		{
			return m_pComponentObject->GetData<TComponent>();
		}

		virtual void Initialize() override
		{
			m_pComponentObject = (EntityComponentObject*)m_pTarget;
		}

		virtual bool OnGUI() override
		{
			Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
			bool change = false;

			const uint32_t hash = ResourceType::GetHash<TComponent>();
			const TypeData* pTypeData = Reflect::GetTyeData(hash);

			if (pTypeData)
			{
				const Utils::ECS::EntityID entity = m_pComponentObject->EntityID();
				Utils::ECS::BaseTypeView* pTypeView = m_pComponentObject->GetRegistry()->GetTypeView(hash);
				bool active = pTypeView->IsActive(entity);
				if (EditorUI::CheckBox("Active", &active))
				{
					pTypeView->SetActive(entity, active);
					change = true;
				}

				TComponent& component = GetTargetComponent();
				PropertyDrawer::DrawProperty("", pTypeData, &component, 0);
			}

			Undo::StopRecord();

			if (change) Validate();
			return change;
		}

	protected:
		void Validate()
		{
			TComponent& component = GetTargetComponent();
			Utils::ECS::EntityRegistry* pRegistry = m_pComponentObject->GetRegistry();
			Utils::ECS::TypeView<TComponent>* pTypeView = pRegistry->GetTypeView<TComponent>();
			pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, pRegistry, m_pComponentObject->EntityID(), &component);
		}

	protected:
		EntityComponentObject* m_pComponentObject;
	};
}
