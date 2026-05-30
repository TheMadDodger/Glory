#pragma once
#include "Editor.h"
#include "PropertyDrawer.h"
#include "EditorUI.h"
#include "EntityComponentObject.h"

#include <ComponentManager.h>

#include <Object.h>
#include <TypeData.h>

namespace Glory::Editor
{
	template<typename TEditor, typename TComponent>
	class EntityComponentEditor : public EditorTemplate<TEditor, EntityComponentObject>
	{
	public:
		EntityComponentEditor() : EditorTemplate<TEditor, EntityComponentObject>(ResourceTypes::GetHash<TComponent>()), m_pComponentObject(nullptr) {}
		virtual ~EntityComponentEditor() {}

	protected:
		TComponent& GetTargetComponent()
		{
			return m_pComponentObject->GetData<TComponent>();
		}

		virtual void Initialize() override
		{
			m_pComponentObject = (EntityComponentObject*)Editor::m_pTarget;
		}

		virtual bool OnGUI() override
		{
			Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
			bool change = false;

			const uint32_t hash = ResourceTypes::GetHash<TComponent>();
			const TypeData* pTypeData = Reflect::GetTyeData(hash);

			if (pTypeData)
			{
				const Utils::ECS::EntityID entity = m_pComponentObject->EntityID();
				Utils::ECS::IComponentManager* manager = m_pComponentObject->GetRegistry()->GetComponentManager(hash);
				bool active = manager->IsActive(entity);
				if (EditorUI::CheckBox("Active", &active))
				{
					if (active)
						manager->Activate(entity);
					else
						manager->Deactivate(entity);
					change = true;
				}

				TComponent& component = GetTargetComponent();
				change |= PropertyDrawer::DrawProperty("", pTypeData, &component, 0);
			}

			Undo::StopRecord();

			if (change) Validate();
			return change;
		}

	protected:
		void Validate()
		{
			Utils::ECS::EntityRegistry* pRegistry = m_pComponentObject->GetRegistry();
			Utils::ECS::IComponentManager* manager = pRegistry->GetComponentManager<TComponent>();
			manager->CallValidate(m_pComponentObject->EntityID());
		}

	protected:
		EntityComponentObject* m_pComponentObject;
	};
}
