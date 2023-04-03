#pragma once
#include <Object.h>
#include <EntityComponentObject.h>
#include <PropertyDrawer.h>
#include <TypeData.h>
#include "Editor.h"

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
			Undo::StartRecord("Property Change", m_pComponentObject->GetUUID());
			bool change = false;

			uint32_t hash = ResourceType::GetHash<TComponent>();
			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(hash);
			if (pTypeData)
			{
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
			GloryECS::EntityRegistry* pRegistry = m_pComponentObject->GetRegistry();
			GloryECS::TypeView<TComponent>* pTypeView = pRegistry->GetTypeView<TComponent>();
			pTypeView->Invoke(InvocationType::OnValidate, pRegistry, m_pComponentObject->EntityID(), &component);
		}

	protected:
		EntityComponentObject* m_pComponentObject;
	};
}
