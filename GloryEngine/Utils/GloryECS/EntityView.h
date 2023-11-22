#pragma once
#include "UUID.h"
#include "EntityID.h"

#include <vector>
#include <map>

namespace Glory::Utils::ECS
{
	class EntityRegistry;

	class EntityView
	{
	public:
		EntityView(EntityRegistry* pRegistry);

		size_t ComponentCount() const;
		Glory::UUID ComponentUUIDAt(size_t index) const;
		uint32_t ComponentTypeAt(size_t index) const;
		uint32_t ComponentType(Glory::UUID uuid) const;
		std::map<Glory::UUID, uint32_t>::iterator GetIterator();
		std::map<Glory::UUID, uint32_t>::iterator GetIteratorEnd();
		void SwapComponentIndex(size_t index1, size_t index2);
		void SetComponentIndex(size_t oldIndex, size_t newIndex);

		bool& Active();
		bool& HierarchyActive();
		bool IsActive() const;

		EntityID Parent() const;

		size_t ChildCount() const;
		EntityID Child(size_t index) const;

	private:
		void Add(uint32_t hash, Glory::UUID uuid = Glory::UUID());
		UUID Remove(uint32_t hash);
		void Remove(Glory::UUID uuid);

	private:
		friend class EntityRegistry;
		EntityRegistry* m_pRegistry;
		std::map<Glory::UUID, uint32_t> m_ComponentTypes;
		std::map<uint32_t, Glory::UUID> m_TypeToUUID;
		std::vector<Glory::UUID> m_ComponentOrder;
		EntityID m_Parent;
		std::vector<EntityID> m_Children;
		bool m_Active;
		bool m_HierarchyActive;
	};
}
