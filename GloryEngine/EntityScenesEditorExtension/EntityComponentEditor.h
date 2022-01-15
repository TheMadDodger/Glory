#pragma once
#include <Object.h>
#include "EntityComponentObject.h"
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
		}

	private:
		virtual const std::type_index& GetEditedType() override
		{
			return typeid(TComponent);
		}

	private:
		EntityComponentObject* m_pComponentObject;
	};
}
