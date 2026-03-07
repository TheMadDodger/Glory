#pragma once
#include "EntityID.h"
#include "IComponentManager.h"
#include "ECSTypeTraits.h"

#include <SparseSet.h>
#include <Hash.h>
#include <BitSet.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;

	template<ComponentCompatible Component>
	class ComponentManager : public SparseSet<EntityID, Component>, public IComponentManager
	{
	public:
		ComponentManager(EntityRegistry* pRegistry, size_t capacity=32):
			m_pRegistry(pRegistry), SparseSet<EntityID, Component>{ 1000, capacity },
			m_ComponentEnabled(capacity) {}
		virtual ~ComponentManager() = default;

		static uint32_t GetComponentHash()
		{
			return Hashing::Hash(typeid(Component).name());
		}

		virtual uint32_t ComponentHash() const override
		{
			return ComponentTypeHash;
		}

		virtual void* Add(EntityID entity)
		{
			return &SparseSet<EntityID, Component>::Add(entity, Component());
		}

		virtual Component& Get(EntityID entity)
		{
			return SparseSet<EntityID, Component>::Get(entity);
		}

		void Sort(const std::vector<std::vector<EntityID>>& entityTrees, size_t& currentIndex, EntityID root=0ull) override
		{
			for (size_t i = 0; i < entityTrees[root].size(); ++i)
			{
				const EntityID child = entityTrees[root][i];
				const size_t index = SparseSet<EntityID, Component>::Index(child);
				if (index == SparseSet<EntityID, Component>::InvalidIndex) continue;
				SparseSet<EntityID, Component>::Swap(currentIndex, index);
				++currentIndex;
				Sort(entityTrees, currentIndex, child);
			}
		}

		virtual void Initialize()
		{
			OnInitialize();
		}

	protected:
		virtual void OnInitialize() {}
		virtual void OnAddComponent(EntityID, Component&) {}
		virtual void OnRemoveComponent(EntityID, size_t) {}
		virtual void OnReserveComponents() {}
		virtual void OnReserveIDs() {}
		virtual void OnSwapComponents(size_t index1, size_t index2) {}

		typedef void (ComponentManager<Component>::* Function)(EntityID, Component&);
		typedef void (ComponentManager<Component>::* UpdateFunction)(EntityID, Component&, float);
		Function DoStart = nullptr;
		Function DoStop = nullptr;
		UpdateFunction DoPreUpdate = nullptr;
		UpdateFunction DoUpdate = nullptr;
		UpdateFunction DoPostUpdate = nullptr;
		Function DoPreDraw = nullptr;
		Function DoDraw = nullptr;
		Function DoPostDraw = nullptr;

		template<typename Manager>
		void Bind(Function& target, void(Manager::*func)(EntityID, Component&))
		{
			target = static_cast<Function>(func);
		}

		template<typename Manager>
		void Bind(UpdateFunction& target, void(Manager::* func)(EntityID, Component&, float))
		{
			target = static_cast<UpdateFunction>(func);
		}

	private:
		virtual void OnAdd(size_t denseIndex, EntityID entity, Component& component) override final
		{
			m_ComponentEnabled.Set(denseIndex);
			OnAddComponent(entity, component);
		}

		virtual void OnRemove(EntityID entity, size_t index) override final
		{
			m_ComponentEnabled.Set(index, false);
			OnRemoveComponent(entity, index);
		}

		virtual void OnReserveDense() override final
		{
			m_ComponentEnabled.Reserve(SparseSet<EntityID, Component>::DenseCapacity());
			OnReserveComponents();
		}

		virtual void OnReserveSparse() override final
		{
			OnReserveIDs();
		}

		virtual void OnSwap(size_t index1, size_t index2) override final
		{
			const bool enabled1 = m_ComponentEnabled.IsSet(index1);
			const bool enabled2 = m_ComponentEnabled.IsSet(index2);
			m_ComponentEnabled.Set(index1, enabled2);
			m_ComponentEnabled.Set(index2, enabled1);
			OnSwapComponents(index1, index2);
		}

		virtual void Start() override final
		{
			if (!DoStart) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoStart)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Stop() override final
		{
			if (!DoStop) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoStop)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void PreUpdate(float dt) override final
		{
			if (!DoPreUpdate) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoPreUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void Update(float dt) override final
		{
			if (!DoUpdate) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PostUpdate(float dt) override final
		{
			if (!DoPostUpdate) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoPostUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PreDraw() override final
		{
			if (!DoPreDraw) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoPreDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Draw() override final
		{
			if (!DoDraw) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void PostDraw() override final
		{
			if (!DoPostDraw) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				if (!m_ComponentEnabled.IsSet(i)) continue;
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				if (!m_pRegistry->EntityActive(entity)) continue;
				(this->*DoPostDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

	protected:
		const uint32_t ComponentTypeHash = Hashing::Hash(typeid(Component).name());
		EntityRegistry* m_pRegistry;
		BitSet m_ComponentEnabled;
	};
}
