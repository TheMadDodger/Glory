#pragma once
#include <Object.h>
#include <EntityComponentObject.h>
#include <EntitySystems.h>
#include <PropertyDrawer.h>
#include <TypeData.h>
#include "Editor.h"

namespace Glory::Editor
{
	template<typename TEditor, typename TComponent>
	class EntityComponentEditor : public EditorTemplate<TEditor, EntityComponentObject>
	{
	public:
		EntityComponentEditor() : m_pComponentObject(nullptr) {}
		virtual ~EntityComponentEditor() {}

	protected:
		TComponent& GetTargetComponent()
		{
			return m_pComponentObject->GetData<TComponent>();
		}

		virtual void Initialize() override
		{
			m_pComponentObject = (EntityComponentObject*)m_pTarget;
			m_Properties.clear();
		}

		virtual bool OnGUI() override
		{
			Undo::StartRecord("Property Change", m_pComponentObject->GetUUID());
			bool change = false;
			TComponent& component = GetTargetComponent();
			size_t hash = ResourceType::GetHash<TComponent>();

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(hash);
			if (pTypeData)
			{
				for (size_t i = 0; i < pTypeData->FieldCount(); i++)
				{
					const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
					size_t offset = pFieldData->Offset();
					void* pAddress = (void*)((char*)(&component) + offset);
					change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, 0);
				}
			}

			Undo::StopRecord();
			return change;
		}

	private:
		virtual const std::type_index& GetEditedType() override
		{
			return typeid(TComponent);
		}

	private:
		EntityComponentObject* m_pComponentObject;
		std::vector<SerializedProperty*> m_Properties;
	};
}
