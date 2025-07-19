#pragma once
#include "UUID.h"

#include <BitSet.h>
#include <map>
#include <functional>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
	class BaseTypeView;

	enum class InvocationType
	{
		OnAdd,
		OnRemove,
		OnValidate,
		OnEnable,
		OnDisable,
		OnEnableDraw,
		OnDisableDraw,
		OnDirty,
		Start,
		Stop,
		PreUpdate,
		Update,
		PostUpdate,
		Draw,
		PostDraw,

		Count
	};

	template<typename T>
	class ComponentInvokations
	{
	public:
		ComponentInvokations()
		{
			for (size_t i = 0; i < (size_t)InvocationType::Count; i++)
			{
				m_Callbacks[InvocationType(i)] =  NULL;
			}
		}
		virtual ~ComponentInvokations() { m_Callbacks.clear(); }

		void Invoke(const InvocationType& invocationType, EntityRegistry* pRegistry, EntityID entity, T& component)
		{
			if (!pRegistry->CallbacksEnabled()
				|| !pRegistry->CallbackEnabled(invocationType)
				|| !m_Callbacks[invocationType]) return;
			m_Callbacks[invocationType](pRegistry, entity, component);
		}

		void InvokeReferencesCallback(const BaseTypeView* pTypeView, std::vector<UUID>& references)
		{
			if (!m_ReferencesCallback) return;
			m_ReferencesCallback(pTypeView, references);
		}

	private:
		friend class EntityRegistry;
		friend class ComponentTypes;
		std::map<InvocationType, std::function<void(EntityRegistry*, EntityID, T&)>> m_Callbacks;
		std::function<void(const BaseTypeView*, std::vector<UUID>&)> m_ReferencesCallback;
	};
}
