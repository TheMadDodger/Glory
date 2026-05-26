#pragma once
#include "EntityID.h"
#include "IComponentManager.h"
#include "ECSTypeTraits.h"
#include "EntityCallType.h"

#include <SparseSet.h>
#include <BinaryStream.h>
#include <Hash.h>
#include <BitSet.h>
#include <UUID.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;

	template<ComponentCompatible Component>
	class ComponentManager : public SparseSet<EntityID, Component>, public IComponentManager
	{
	public:
		ComponentManager(EntityRegistry* pRegistry, size_t capacity=32) :
			m_pRegistry(pRegistry), SparseSet<EntityID, Component>{ 1000, capacity },
			m_ComponentManagerIndex(0ull), m_ComponentActive(capacity), m_ActiveSize(0ull) { }
		virtual ~ComponentManager() = default;

		static uint32_t GetComponentHash()
		{
			return Hashing::Hash(typeid(Component).name());
		}

		virtual void Initialize(size_t componentManagerIndex)
		{
			m_ComponentManagerIndex = componentManagerIndex;
			OnInitialize();
		}

		virtual uint32_t ComponentHash() const override
		{
			return ComponentTypeHash;
		}

		template<typename... Args>
		Component& AddInPlace(EntityID entity, Args&&... args)
		{
			Component c(args...);
			return SparseSet<EntityID, Component>::Add(entity, std::move(c));
		}

		virtual void* Add(EntityID entity) override
		{
			return &SparseSet<EntityID, Component>::Add(entity, Component());
		}

		virtual void UnpackDataInto(const void* data, Component& newComponent)
		{
			std::memcpy(&newComponent, data, sizeof(Component));
		}

		virtual void* Add(EntityID entity, const void* data) override
		{
			Component newComponent = Component();
			UnpackDataInto(data, newComponent);
			return &SparseSet<EntityID, Component>::Add(entity, std::move(newComponent));
		}

		virtual void Remove(EntityID entity) override
		{
			SparseSet<EntityID, Component>::Remove(entity);
		}

		virtual void* GetAddress(EntityID entity) override
		{
			return &SparseSet<EntityID, Component>::Get(entity);
		}

		virtual const void* GetAddress(EntityID entity) const override
		{
			return static_cast<const void*>(&SparseSet<EntityID, Component>::Get(entity));
		}

		virtual size_t NumComponents() override
		{
			return SparseSet<EntityID, Component>::Size();
		}

		virtual EntityID EntityAt(size_t index) override
		{
			return SparseSet<EntityID, Component>::DenseID(index);
		}

		Component& Get(EntityID entity)
		{
			return SparseSet<EntityID, Component>::Get(entity);
		}

		virtual void Clear() override
		{
			SparseSet<EntityID, Component>::Clear();
		}

		void Sort(const std::vector<std::vector<EntityID>>& entityTrees) override
		{
			size_t currentIndex = 0;
			SortRecursive(entityTrees, currentIndex);
			m_ActiveSize = currentIndex;
		}

		virtual bool IsActive(EntityID entity) override
		{
			const size_t index = SparseSet<EntityID, Component>::Index(entity);
			if (index == SparseSet<EntityID, Component>::InvalidIndex) return false;
			return m_ComponentActive.IsSet(index);
		}

		virtual size_t ActiveSize() const override
		{
			return m_ActiveSize;
		}

		virtual void Activate(EntityID entity) override
		{
			const size_t index = SparseSet<EntityID, Component>::Index(entity);
			if (index == SparseSet<EntityID, Component>::InvalidIndex) return;
			const bool wasActive = m_ComponentActive.IsSet(index);
			if (wasActive) return;
			m_ComponentActive.Set(index);
			const bool entityActive = m_pRegistry->EntityActiveHierarchy(entity);
			m_pRegistry->SetComponentOrderDirtyAt(m_ComponentManagerIndex);
			if (!entityActive) return;
			CallOnActivate(entity);
			CallOnEnableDraw(entity);
		}

		virtual void Deactivate(EntityID entity) override
		{
			const size_t index = SparseSet<EntityID, Component>::Index(entity);
			if (index == SparseSet<EntityID, Component>::InvalidIndex) return;
			const bool wasActive = m_ComponentActive.IsSet(index);
			if (!wasActive) return;
			m_ComponentActive.UnSet(index);
			const bool entityActive = m_pRegistry->EntityActiveHierarchy(entity);
			m_pRegistry->SetComponentOrderDirtyAt(m_ComponentManagerIndex);
			if (!entityActive) return;
			CallOnDeactivate(entity);
			CallOnDisableDraw(entity);
		}

		virtual void GetReferences(std::vector<UUID>& references) const override
		{
			if (!DoGetReferences) return;
			(this->*DoGetReferences)(references);
		}

		virtual void Serialize(BinaryStream& stream) const override
		{
			GrowableBinaryMemoryStream memoryStream;
			BinaryStream* tempStream = &memoryStream;

			tempStream->Write(SparseSet<EntityID, Component>::Size());
			SerializeDense(*tempStream);
			tempStream->Write(SparseSet<EntityID, Component>::DenseIDData(), SparseSet<EntityID, Component>::Size()*sizeof(EntityID))
				.Write(SparseSet<EntityID, Component>::SparseCapacity()).Write(SparseSet<EntityID, Component>::SparseData());

			tempStream->Write(m_ComponentActive).Write(m_ActiveSize);
			OnSerialize(*tempStream);

			stream.Write(ComponentTypeHash).Write(memoryStream.Tell()).Write(memoryStream.Buffer(), memoryStream.Tell());
		}

		virtual void Deserialize(BinaryStream& stream) override
		{
			size_t denseSize;
			stream.Read(denseSize);
			SparseSet<EntityID, Component>::ResizeDense(denseSize);

			DeserializeDense(stream);

			size_t sparseCapacity;
			stream.Read(SparseSet<EntityID, Component>::DenseIDData(), SparseSet<EntityID, Component>::Size()*sizeof(EntityID))
				.Read(sparseCapacity);
			SparseSet<EntityID, Component>::ReserveSparse(sparseCapacity);

			stream.Read(SparseSet<EntityID, Component>::SparseData());
			SparseSet<EntityID, Component>::DoneReading();

			stream.Read(m_ComponentActive).Read(m_ActiveSize);
			OnDeserialize(stream);
		}

		virtual bool Compare(const IComponentManager* other) const override
		{
			if (ComponentTypeHash != other->ComponentHash()) return false;
			const ComponentManager<Component>* otherManager = static_cast<const ComponentManager<Component>*>(other);
			if (m_ComponentActive != otherManager->m_ComponentActive) return false;
			if (m_ActiveSize != otherManager->m_ActiveSize) return false;
			if (SparseSet<EntityID, Component>::Size() != otherManager->SparseSet<EntityID, Component>::Size()) return false;
			if (std::memcmp(SparseSet<EntityID, Component>::DenseData(), otherManager->SparseSet<EntityID, Component>::DenseData(),
				SparseSet<EntityID, Component>::Size()*sizeof(Component)) != 0) return false;
			if (std::memcmp(SparseSet<EntityID, Component>::DenseIDData(), otherManager->SparseSet<EntityID, Component>::DenseIDData(),
				SparseSet<EntityID, Component>::Size()*sizeof(EntityID)) != 0) return false;
			return SparseSet<EntityID, Component>::SparseData() == otherManager->SparseSet<EntityID, Component>::SparseData();
		}

		virtual size_t IndexOf(EntityID entity) const override
		{
			return SparseSet<EntityID, Component>::Index(entity);
		}

		virtual std::type_index ComponentType() const override
		{
			return typeid(Component);
		}

		virtual void SetComponentActive(EntityID entity, bool active) override
		{
			if (m_ComponentActive.IsSet(entity) == active) return;
			m_ComponentActive.Set(entity, active);
			m_pRegistry->SetComponentOrderDirty(ComponentTypeHash);
		}

	protected: /* Custom implementations, these are always called */
		virtual void OnInitialize() {}
		virtual void OnAddComponent(EntityID, Component&) {}
		virtual void OnRemoveComponent(EntityID, size_t) {}
		virtual void OnReserveComponents() {}
		virtual void OnReserveIDs() {}
		virtual void OnSwapComponents(size_t index1, size_t index2) {}
		virtual void OnSerialize(BinaryStream&) const {}
		virtual void OnDeserialize(BinaryStream&) {}

		virtual void SerializeDense(BinaryStream& stream) const
		{
			stream.Write(SparseSet<EntityID, Component>::DenseData(), SparseSet<EntityID, Component>::Size()*sizeof(Component));
		}

		virtual void DeserializeDense(BinaryStream& stream)
		{
			stream.Read(SparseSet<EntityID, Component>::DenseData(), SparseSet<EntityID, Component>::Size()*sizeof(Component));
		}

	protected: /* Component callbacks */
		typedef void (ComponentManager<Component>::*Function)(EntityID, Component&);
		typedef void (ComponentManager<Component>::*UpdateFunction)(EntityID, Component&, float);
		typedef void (ComponentManager<Component>::*ReferencesFunction)(std::vector<UUID>&) const;
		Function DoValidate = nullptr;
		Function DoOnAdd = nullptr;
		Function DoOnRemove = nullptr;
		Function DoOnActivate = nullptr;
		Function DoOnDeactivate = nullptr;
		Function DoOnEnableDraw = nullptr;
		Function DoOnDisableDraw = nullptr;
		Function DoStart = nullptr;
		Function DoStop = nullptr;
		Function DoOnDirty = nullptr;
		UpdateFunction DoPreUpdate = nullptr;
		UpdateFunction DoUpdate = nullptr;
		UpdateFunction DoPostUpdate = nullptr;
		Function DoPreDraw = nullptr;
		Function DoDraw = nullptr;
		Function DoPostDraw = nullptr;
		ReferencesFunction DoGetReferences = nullptr;

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

		template<typename Manager>
		void Bind(ReferencesFunction& target, void(Manager::*func)(std::vector<UUID>&) const)
		{
			target = static_cast<ReferencesFunction>(func);
		}

	private:
		virtual void OnAdd(size_t denseIndex, EntityID entity, Component& component) override final
		{
			m_ComponentActive.Set(denseIndex);
			++m_ActiveSize;
			OnAddComponent(entity, component);
			CallOnAdd(entity);
		}

		virtual void OnRemove(EntityID entity, size_t index) override final
		{
			const bool wasActive = m_ComponentActive.IsSet(index) && m_pRegistry->EntityActiveHierarchy(entity);
			m_ComponentActive.Set(index, false);
			if (wasActive) --m_ActiveSize;
			OnRemoveComponent(entity, index);
			CallOnRemove(entity);
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

		virtual void OnClear() override final
		{
			m_ComponentActive.Clear();
			m_ActiveSize = 0;
		}

	private: /* Global component callbacks */
		virtual void Dirty() override final
		{
			if (!DoOnDirty) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnDirty)) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoOnDirty)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Validate() override final
		{
			if (!DoValidate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnValidate)) return;

			const size_t numComponents = SparseSet<EntityID, Component>::Size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoValidate)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Activate() override final
		{
			if (!DoOnActivate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnActivate)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoOnActivate)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Deactivate() override final
		{
			if (!DoOnDeactivate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnActivate)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoOnDeactivate)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void EnableDraw() override final
		{
			if (!DoOnEnableDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnEnableDraw)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoOnEnableDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void DisableDraw() override final
		{
			if (!DoOnDisableDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnDisableDraw)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoOnDisableDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Start() override final
		{
			if (!DoStart) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Start)) return;

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
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Stop)) return;

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
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PreUpdate)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPreUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void Update(float dt) override final
		{
			if (!DoUpdate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Update)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PostUpdate(float dt) override final
		{
			if (!DoPostUpdate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PostUpdate)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPostUpdate)(entity, SparseSet<EntityID, Component>::GetAt(i), dt);
			}
		}

		virtual void PreDraw() override final
		{
			if (!DoPreDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PreDraw)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoPreDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void Draw() override final
		{
			if (!DoDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Draw)) return;

			for (size_t i = 0; i < m_ActiveSize; ++i)
			{
				const EntityID entity = SparseSet<EntityID, Component>::DenseID(i);
				(this->*DoDraw)(entity, SparseSet<EntityID, Component>::GetAt(i));
			}
		}

		virtual void PostDraw() override final
		{
			if (!DoPostDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PostDraw)) return;

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
				if (index == SparseSet<EntityID, Component>::InvalidIndex)
				{
					SortRecursive(entityTrees, currentIndex, child);
					continue;
				}
				if (!m_ComponentActive.IsSet(index) || !m_pRegistry->EntityActiveHierarchy(child)) continue;
				SparseSet<EntityID, Component>::Swap(index, currentIndex);
				++currentIndex;
				SortRecursive(entityTrees, currentIndex, child);
			}
		}

	private: /* Manual calls */
		virtual void CallValidate(EntityID entity) override
		{
			if (!DoValidate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnValidate)) return;
			(this->*DoValidate)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnAdd(EntityID entity) override
		{
			if (!DoOnAdd) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnAdd)) return;
			(this->*DoOnAdd)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnRemove(EntityID entity) override
		{
			if (!DoOnRemove) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnRemove)) return;
			(this->*DoOnRemove)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnActivate(EntityID entity) override
		{
			if (!DoOnActivate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnActivate)) return;
			(this->*DoOnActivate)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnDeactivate(EntityID entity) override
		{
			if (!DoOnDeactivate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnDeactivate)) return;
			(this->*DoOnDeactivate)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnEnableDraw(EntityID entity) override
		{
			if (!DoOnEnableDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnEnableDraw)) return;
			(this->*DoOnEnableDraw)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnDisableDraw(EntityID entity) override
		{
			if (!DoOnDisableDraw) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnDisableDraw)) return;
			(this->*DoOnDisableDraw)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallStart(EntityID entity) override
		{
			if (!DoStart) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Start)) return;
			(this->*DoStart)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallStop(EntityID entity) override
		{
			if (!DoStop) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Stop)) return;
			(this->*DoStop)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallOnDirty(EntityID entity) override
		{
			if (!DoOnDirty) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::OnDirty)) return;
			(this->*DoOnDirty)(entity, SparseSet<EntityID, Component>::Get(entity));
		}

		virtual void CallPreUpdate(EntityID entity, float dt) override
		{
			if (!DoPreUpdate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PreUpdate)) return;
			(this->*DoPreUpdate)(entity, SparseSet<EntityID, Component>::Get(entity), dt);
		}

		virtual void CallUpdate(EntityID entity, float dt) override
		{
			if (!DoUpdate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::Update)) return;
			(this->*DoUpdate)(entity, SparseSet<EntityID, Component>::Get(entity), dt);
		}

		virtual void CallPostUpdate(EntityID entity, float dt) override
		{
			if (!DoPostUpdate) return;
			if (!m_pRegistry->IsCallEnabled(EntityCallType::PostUpdate)) return;
			(this->*DoPostUpdate)(entity, SparseSet<EntityID, Component>::Get(entity), dt);
		}

	protected:
		const uint32_t ComponentTypeHash = Hashing::Hash(typeid(Component).name());
		EntityRegistry* m_pRegistry;
		size_t m_ComponentManagerIndex;
		BitSet m_ComponentActive;
		size_t m_ActiveSize;
	};
}
