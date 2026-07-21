#pragma once
#include "EntityID.h"
#include "IComponentManager.h"
#include "ECSTypeTraits.h"
#include "EntityCallType.h"

#include <Hash.h>
#include <UUID.h>
#include <BitSet.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <cassert>

namespace Glory::Utils
{
	class BinaryStream;
}

namespace Glory::Utils::ECS
{
	template<ComponentCompatible Component>
	class ComponentManager;

	class EntityRegistry
	{
	public:
		EntityRegistry(void* userData=nullptr, size_t reserveComponentManagers=1, size_t reserveEntities=100);
		/** @overload */
		EntityRegistry(EntityRegistry&& other) noexcept;
		/** @brief Move assignment operator */
		EntityRegistry& operator=(EntityRegistry&& other) noexcept;
		virtual ~EntityRegistry();

		EntityID CreateEntity();

		template<IsComponentManager Manager>
		Manager& AddManager()
		{
			const uint32_t hash = Manager::GetComponentHash();
			const uint32_t index = uint32_t(m_ComponentManagers.size());
			auto& newManager = m_ComponentManagers.emplace_back(new Manager(this));
			m_HashToComponentManagerIndex.emplace(hash, index);
			m_ComponentOrderDirty.Reserve(index + 1ull);
			m_ComponentOrderDirty.Set(index, false);
			newManager->Initialize(index);
			return static_cast<Manager&>(*newManager);
		}

		void* CreateComponent(EntityID entity, uint32_t componentHash, UUID componentID=UUID());

		void AddManager(IComponentManager* manager);

		template<ComponentCompatible Component, typename... Args>
		Component& AddComponent(EntityID entity, UUID componentID, Args&&... args)
		{
			size_t index = 0;
			ComponentManager<Component>* manager = GetComponentManager<Component>(&index);
			m_ComponentOrderDirty.Set(index);
			m_HasComponent[entity].Set(index);
			m_EntityComponentOrder[entity].emplace_back(manager->ComponentHash(), componentID);
			return manager->AddInPlace(entity, std::forward<Args>(args)...);
		}

		template<ComponentCompatible Component>
		UUID RemoveComponent(EntityID entity)
		{
			size_t index = 0;
			ComponentManager<Component>* manager = GetComponentManager<Component>(&index);
			assert(m_HasComponent[entity].IsSet(index));
			auto iter = std::find_if(m_EntityComponentOrder[entity].begin(), m_EntityComponentOrder[entity].end(),
				[manager](const std::pair<uint32_t, UUID>& pair) {
					return pair.first == manager->ComponentHash();
				});

			manager->Remove(entity);
			const UUID id = iter->second;
			m_EntityComponentOrder[entity].erase(iter);
			m_HasComponent[entity].UnSet(index);
			return id;
		}

		UUID RemoveComponent(EntityID entity, uint32_t typeHash);
		UUID RemoveComponentAt(EntityID entity, size_t index);

		template<ComponentCompatible Component>
		Component& GetComponent(EntityID entity)
		{
			ComponentManager<Component>* manager = GetComponentManager<Component>();
			return manager->Get(entity);
		}

		template<ComponentCompatible Component>
		Component& GetComponent(EntityID entity) const
		{
			const ComponentManager<Component>* manager = GetComponentManager<Component>();
			return manager->Get(entity);
		}

		template<ComponentCompatible Component>
		bool HasComponent(EntityID entity) const
		{
			size_t index = 0;
			GetComponentManager<Component>(&index);
			return m_HasComponent[entity].IsSet(index);
		}

		bool HasComponent(EntityID entity, uint32_t typeHash) const;

		template<ComponentCompatible Component>
		ComponentManager<Component>* GetComponentManager(size_t* outIndex=nullptr)
		{
			const uint32_t hash = Hashing::Hash(typeid(Component).name());
			return static_cast<ComponentManager<Component>*>(GetComponentManager(hash, outIndex));
		}

		template<ComponentCompatible Component>
		const ComponentManager<Component>* GetComponentManager(size_t* outIndex = nullptr) const
		{
			const uint32_t hash = Hashing::Hash(typeid(Component).name());
			return static_cast<const ComponentManager<Component>*>(GetComponentManager(hash, outIndex));
		}

		size_t ComponentManagerCount() const;
		IComponentManager* GetComponentManager(uint32_t componentHash, size_t* outIndex=nullptr);
		const IComponentManager* GetComponentManager(uint32_t componentHash, size_t* outIndex=nullptr) const;
		IComponentManager& GetComponentManagerAt(size_t index);
		const IComponentManager& GetComponentManagerAt(size_t index) const;

		bool EntityValid(EntityID entity) const;
		bool EntityActiveHierarchy(EntityID entity) const;
		bool EntityActiveSelf(EntityID entity) const;

		EntityID GetParent(EntityID entity) const;
		void SetParent(EntityID entity, EntityID parent);

		void SetActive(EntityID entity, bool active, bool withCallbacks=true);

		void DestroyEntity(EntityID entity);
		void Clear(EntityID entity);

		bool IsEntityDirty(EntityID entity) const;
		void SetEntityDirty(EntityID entity, bool dirty=true, bool setChildrenDirty=true, bool withCallbacks=true);

		size_t ChildCount(EntityID entity) const;
		EntityID Child(EntityID entity, size_t index) const;
		size_t SiblingIndex(EntityID entity) const;
		void SetSiblingIndex(EntityID entity, size_t index);

		size_t EntityComponentCount(EntityID entity) const;
		uint32_t EntityComponentType(EntityID entity, size_t index) const;
		UUID EntityComponentID(EntityID entity, size_t index) const;
		uint32_t EntityComponentIDToHash(EntityID entity, UUID id) const;
		UUID EntityComponentHashToID(EntityID entity, uint32_t typeHash) const;
		void* GetComponentAddress(EntityID entity, uint32_t type);
		const void* GetComponentAddress(EntityID entity, uint32_t type) const;
		void* CopyComponent(EntityID entity, uint32_t type, UUID componentID, const void* data);
		EntityID CopyEntityToOtherRegistry(EntityID entity, EntityID parent, EntityRegistry* pRegistry) const;
		void SetComponentIndex(EntityID entity, size_t from, size_t to);

		template<typename T>
		T* GetUserData()
		{
			return reinterpret_cast<T*>(m_pUserData);
		}

		void Sort();

		void SetUserData(void* data);

		void EnableCalls();
		void DisableCalls();
		void EnableAllIndividualCalls();

		size_t AliveCount() const;

		void GetReferences(std::vector<UUID>& references) const;

		void Serialize(BinaryStream& container) const;
		void Deserialize(BinaryStream& container);

		bool operator==(const EntityRegistry& other) const;

		const std::type_index GetComponentType(uint32_t typeHash) const;

		void Reset();

		void SetComponentOrderDirty(uint32_t typeHash);
		void SetComponentOrderDirtyAt(size_t index);

		inline EntityID MaxEntityID() const
		{
			return m_NextEntityID;
		}

	public: /* Global calls */
		void Dirty();
		void Validate();
		void Activate();
		void Deactivate();
		void EnableDraw();
		void DisableDraw();
		void Start();
		void Stop();
		void Update(float dt);
		void Draw();

		template<ComponentCompatible Component>
		void CallStart()
		{
			IComponentManager* manager = GetComponentManager<Component>();
			manager->Start();
		}

	public: /* Individual calls */
		void CallOnValidate(EntityID entity);
		void CallStart(EntityID entity);
		void CallOnActivate(EntityID entity);
		void CallOnEnableDraw(EntityID entity);

	public:
		void EnableCall(EntityCallType callType, bool enable=true);
		bool IsCallEnabled(EntityCallType callType) const;

	private:
		void SetHierarchyActiveStateChildren(EntityID entity, bool active, bool withCallbacks=true);

	private:
		std::vector<std::unique_ptr<IComponentManager>> m_ComponentManagers;
		std::unordered_map<uint32_t, uint32_t> m_HashToComponentManagerIndex;
		BitSet m_ComponentOrderDirty;

		BitSet m_EntityAlive;
		BitSet m_EntityActiveSelf;
		BitSet m_EntityActiveHierarchy;
		BitSet m_EntityDirty;

		std::vector<std::vector<EntityID>> m_EntityTrees;
		std::vector<EntityID> m_Parents;
		std::vector<BitSet> m_HasComponent;
		std::vector<std::vector<std::pair<uint32_t, UUID>>> m_EntityComponentOrder;

		EntityID m_NextEntityID;
		size_t m_AliveCount;

		BitSet m_EnabledCalls;

		void* m_pUserData;

		bool m_CallsEnabled = true;
	};
}
