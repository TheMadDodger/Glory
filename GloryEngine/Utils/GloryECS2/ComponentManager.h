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
			m_ComponentActive(capacity), m_ActiveSize(0) {}
		virtual ~ComponentManager() = default;

		static uint32_t GetComponentHash()
		{
			return Hashing::Hash(typeid(Component).name());
		}

		virtual uint32_t ComponentHash() const override
		{
			return ComponentTypeHash;
		}

		virtual void* Add(EntityID entity) override
		{
			return &SparseSet<EntityID, Component>::Add(entity, Component());
		}

		virtual void Remove(EntityID entity) override
		{
			SparseSet<EntityID, Component>::Remove(entity);
		}

		Component& Get(EntityID entity)
		{
			return SparseSet<EntityID, Component>::Get(entity);
		}

		void Sort(const std::vector<std::vector<EntityID>>& entityTrees) override
		{
			size_t currentIndex = 0;
			SortRecursive(entityTrees, currentIndex);
			m_ActiveSize = currentIndex;
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
			m_ComponentActive.Set(denseIndex);
			++m_ActiveSize;
			OnAddComponent(entity, component);
		}

		virtual void OnRemove(EntityID entity, size_t index) override final
		{
			m_ComponentActive.Set(index, false);
			--m_ActiveSize;
			OnRemoveComponent(entity, index);
		}

		virtual void OnReserveDense() override final
		{
			m_ComponentActive.Reserve(SparseSet<EntityID, Component>::DenseCapacity());
			OnReserveComponents();
		}

		virtual void OnReserveSparse() override final
		{
			OnReserveIDs();
		}

		virtual void OnSwap(size_t index1, size_t index2) override final
		{
			const bool enabled1 = m_ComponentActive.IsSet(index1);
			const bool enabled2 = m_ComponentActive.IsSet(index2);
			m_ComponentActive.Set(index1, enabled2);
			m_ComponentActive.Set(index2, enabled1);
			OnSwapComponents(index1, index2);
		}

		virtual void Start() override final
		{
			if (!DoStart) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoStart)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Stop() override final
		{
			if (!DoStop) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoStop)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void PreUpdate(float dt) override final
		{
			if (!DoPreUpdate) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPreUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void Update(float dt) override final
		{
			if (!DoUpdate) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PostUpdate(float dt) override final
		{
			if (!DoPostUpdate) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPostUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PreDraw() override final
		{
			if (!DoPreDraw) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPreDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Draw() override final
		{
			if (!DoDraw) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void PostDraw() override final
		{
			if (!DoPostDraw) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPostDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		void SortRecursive(const std::vector<std::vector<EntityID>>& entityTrees, size_t& currentIndex, EntityID root=0ull)
		{
			for (size_t i = 0; i < entityTrees[root].size(); ++i)
			{
				const EntityID child = entityTrees[root][i];
				const size_t index = SparseSet<EntityID, Component>::Index(child);
				if (index == SparseSet<EntityID, Component>::InvalidIndex) continue;
				if (!m_ComponentActive.IsSet(i) || !m_pRegistry->EntityActive(child)) continue;
				SparseSet<EntityID, Component>::Swap(currentIndex, index);
				++currentIndex;
				SortRecursive(entityTrees, currentIndex, child);
			}
		}

	protected:
		const uint32_t ComponentTypeHash = Hashing::Hash(typeid(Component).name());
		EntityRegistry* m_pRegistry;
		BitSet m_ComponentActive;
		size_t m_ActiveSize;
	};
}
