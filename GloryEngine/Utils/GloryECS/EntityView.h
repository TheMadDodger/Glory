#pragma once
#include <vector>
#include <map>
#include <UUID.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;

	class EntityView
	{
	public:
		EntityView(EntityRegistry* pRegistry);

		size_t ComponentCount();
		Glory::UUID ComponentUUIDAt(size_t index);
		uint32_t ComponentTypeAt(size_t index);
		uint32_t ComponentType(Glory::UUID uuid);
		std::map<Glory::UUID, uint32_t>::iterator GetIterator();
		std::map<Glory::UUID, uint32_t>::iterator GetIteratorEnd();
		void SwapComponentIndex(size_t index1, size_t index2);
		void SetComponentIndex(size_t oldIndex, size_t newIndex);

		bool& Active();
		bool& HierarchyActive();
		bool IsActive();

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
		bool m_Active;
		bool m_HierarchyActive;
	};
}
