#pragma once
#include <Object.h>
#include <EntityComponentObject.h>
#include <EntitySystems.h>
#include <PropertyDrawer.h>
#include <Registry.h>
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
			m_pComponentObject->GetRegistry()->GetSystems()->AcquireSerializedProperties(m_pComponentObject->GetComponentData(), m_Properties);
		}

		virtual bool OnGUI() override
		{
			bool change = false;
			for (size_t i = 0; i < m_Properties.size(); i++)
			{
				change |= PropertyDrawer::DrawProperty(m_Properties[i]);
			}
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
