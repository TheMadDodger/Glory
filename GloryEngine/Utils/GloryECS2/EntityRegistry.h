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
		EntityRegistry(size_t reserveComponentManager=1);
		virtual ~EntityRegistry();

		EntityID CreateEntity();

		template<IsComponentManager Manager>
		Manager& AddManager()
		{
			const uint32_t hash = Manager::GetComponentHash();
			const uint32_t index = uint32_t(m_ComponentManagers.size());
			auto& newManager = m_ComponentManagers.emplace_back(new Manager());
			m_HashToComponentManagerIndex.emplace(hash, index);
			newManager->Initialize();
			return static_cast<Manager&>(*newManager);
		}

		void AddManager(IComponentManager* manager)
		{
			const uint32_t hash = manager->ComponentHash();
			const uint32_t index = uint32_t(m_ComponentManagers.size());
			m_ComponentManagers.emplace_back(manager);
			m_HashToComponentManagerIndex.emplace(hash, index);
			manager->Initialize();
		}

		template<ComponentCompatible Component>
		Component& AddComponent(EntityID entity)
		{
			ComponentManager<Component>* manager = GetComponentManager<Component>();
			return *static_cast<Component*>(manager->Add(entity));
		}

		template<ComponentCompatible Component>
		Component& GetComponent(EntityID entity)
		{
			ComponentManager<Component>* manager = GetComponentManager<Component>();
			return manager->Get(entity);
		}

		template<typename Component>
		ComponentManager<Component>* GetComponentManager()
		{
			const uint32_t hash = Hashing::Hash(typeid(Component).name());
			return static_cast<ComponentManager<Component>*>(GetComponentManager(hash));
		}

		IComponentManager* GetComponentManager(uint32_t componentHash);

		bool EntityValid(EntityID entity) const;
		bool EntityActive(EntityID entity) const;

	public:
		void Start();
		void Stop();
		void Update(float dt);
		void Draw();

	private:
		std::vector<std::unique_ptr<IComponentManager>> m_ComponentManagers;
		std::unordered_map<uint32_t, uint32_t> m_HashToComponentManagerIndex;
		BitSet m_EntityAlive;
		BitSet m_EntityActiveSelf;
		BitSet m_EntityActiveHierarchy;

		/* Hierarchy? */

		EntityID m_NextEntityID;
	};
}
