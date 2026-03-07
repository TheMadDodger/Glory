#pragma once
#include "EntityID.h"
#include "IComponentManager.h"
#include "ECSTypeTraits.h"

#include <Hash.h>
#include <BitSet.h>

#include <vector>
#include <memory>
#include <unordered_map>

namespace Glory::Utils::ECS
{
	template<ComponentCompatible Component>
	class ComponentManager;

	class EntityRegistry
	{
	public:
		EntityRegistry(size_t reserveComponentManagers=1, size_t reserveEntities=100);
		virtual ~EntityRegistry();

		EntityID CreateEntity();

		template<IsComponentManager Manager>
		Manager& AddManager()
		{
			const uint32_t hash = Manager::GetComponentHash();
			const uint32_t index = uint32_t(m_ComponentManagers.size());
			auto& newManager = m_ComponentManagers.emplace_back(new Manager());
			m_HashToComponentManagerIndex.emplace(hash, index);
			m_ComponentOrderDirty.Reserve(index + 1ull);
			m_ComponentOrderDirty.Set(index, false);
			newManager->Initialize();
			return static_cast<Manager&>(*newManager);
		}

		void AddManager(IComponentManager* manager);

		template<ComponentCompatible Component>
		Component& AddComponent(EntityID entity)
		{
			size_t index = 0;
			ComponentManager<Component>* manager = GetComponentManager<Component>(&index);
			m_ComponentOrderDirty.Set(index);
			m_HasComponent[entity].Set(index);
			return *static_cast<Component*>(manager->Add(entity));
		}

		template<ComponentCompatible Component>
		void RemoveComponent(EntityID entity)
		{
			size_t index = 0;
			ComponentManager<Component>* manager = GetComponentManager<Component>(&index);
			m_HasComponent[entity].UnSet(index);
			return *static_cast<Component*>(manager->Remove(entity));
		}

		template<ComponentCompatible Component>
		Component& GetComponent(EntityID entity)
		{
			ComponentManager<Component>* manager = GetComponentManager<Component>();
			return manager->Get(entity);
		}

		template<typename Component>
		ComponentManager<Component>* GetComponentManager(size_t* outIndex=nullptr)
		{
			const uint32_t hash = Hashing::Hash(typeid(Component).name());
			return static_cast<ComponentManager<Component>*>(GetComponentManager(hash, outIndex));
		}

		IComponentManager* GetComponentManager(uint32_t componentHash, size_t* outIndex=nullptr);

		bool EntityValid(EntityID entity) const;
		bool EntityActiveHierarchy(EntityID entity) const;
		bool EntityActiveSelf(EntityID entity) const;

		void SetParent(EntityID entity, EntityID parent);

		void Sort();

		void SetActive(EntityID entity, bool active);

	public:
		void Start();
		void Stop();
		void Update(float dt);
		void Draw();

	private:
		void SetHierarchyActiveStateChildren(EntityID entity, bool active);

	private:
		std::vector<std::unique_ptr<IComponentManager>> m_ComponentManagers;
		std::unordered_map<uint32_t, uint32_t> m_HashToComponentManagerIndex;
		BitSet m_ComponentOrderDirty;

		BitSet m_EntityAlive;
		BitSet m_EntityActiveSelf;
		BitSet m_EntityActiveHierarchy;

		std::vector<std::vector<EntityID>> m_EntityTrees;
		std::vector<EntityID> m_Parents;
		std::vector<BitSet> m_HasComponent;

		EntityID m_NextEntityID;
	};
}
