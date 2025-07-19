#pragma once
#include "EntityID.h"

#include "ComponentCallbacks.h"
#include <vector>
#include <exception>
#include <typeindex>
#include <BitSet.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
	class EntityView;

	class BaseTypeView
	{
	public:
		BaseTypeView(uint32_t typeHash, EntityRegistry* pRegistry);
		virtual ~BaseTypeView();

		void Remove(EntityID entityID);
		bool Contains(EntityID entityID);
		const uint32_t ComponentTypeHash() const;
		virtual const std::type_index ComponentType() const = 0;
		virtual void* Create(EntityID entityID) = 0;
		virtual void* Create(EntityID entityID, void* data) = 0;
		virtual void* GetComponentAddress(EntityID entityID, size_t number = 0) = 0;
		virtual const void* GetComponentAddress(EntityID entityID, size_t number = 0) const = 0;
		virtual const void* GetComponentAddressFromIndex(size_t index) const = 0;
		virtual void* GetComponentAddressFromIndex(size_t index) = 0;

		virtual void Invoke(const InvocationType& callbackType, EntityRegistry* pRegistry, EntityID entity, void* pComponentAddress) = 0;
		virtual void InvokeAll(const InvocationType& callbackType, EntityRegistry* pRegistry, std::function<bool(BaseTypeView*, EntityView*, size_t)> canCallCallback) = 0;
		virtual void InvokeAll(const InvocationType& callbackType, EntityRegistry* pRegistry, const std::vector<EntityID>& entities) = 0;

		virtual uint32_t GetComponentIndex(EntityID entityID, size_t number = 0) const;

		bool IsActive(EntityID entity) const;
		bool IsActiveByIndex(size_t index) const;
		void SetActive(EntityID entity, bool active);
		void SetActiveByIndex(size_t index, bool active);

		size_t Size() const;
		EntityID EntityAt(size_t index) const;

		virtual void GetReferences(std::vector<UUID>& references) const {};

		BitSet& ActiveStates();
		const BitSet& ActiveStates() const;

	protected:
		virtual BaseTypeView* Create(EntityRegistry* pRegistry) = 0;
		virtual void OnRemove(size_t index) = 0;

	protected:
		const uint32_t m_TypeHash;
		std::vector<EntityID> m_Entities;
		EntityRegistry* m_pRegistry;
		BitSet m_ActiveStates;

	private:
		friend class EntityRegistry;
		friend class ComponentTypes;
	};

	template<typename T>
	class TypeView : public BaseTypeView
	{
	public:
		TypeView(EntityRegistry* pRegistry) : m_Callbacks(), BaseTypeView(Hashing::Hash(typeid(T).name()), pRegistry) {}
		virtual ~TypeView()
		{
			m_ComponentData.clear();

			if (!m_IsFactory) return;
			delete m_Callbacks;
		}

		template<typename... Args>
		T& Add(EntityID entityID, Args&&... args)
		{
			m_Entities.push_back(entityID);
			size_t index = m_ComponentData.size();
			m_ComponentData.push_back(T(args...));
			m_ActiveStates.Reserve(index);
			m_ActiveStates.Set(index);
			return m_ComponentData[index];
		}

		virtual void OnRemove(size_t index) override
		{
			m_ComponentData.erase(m_ComponentData.begin() + index);
			m_ActiveStates.Set(uint32_t(index));
		}

		T& Get(EntityID entityID)
		{
			auto it = std::find(m_Entities.begin(), m_Entities.end(), entityID);
			if (it == m_Entities.end())
				throw new std::exception("Entity not in view");
			size_t index = it - m_Entities.begin();
			return m_ComponentData[index];
		}

		virtual const std::type_index ComponentType() const override
		{
			return typeid(T);
		}

		virtual void* Create(EntityID entityID) override
		{
			m_Entities.push_back(entityID);
			size_t index = m_ComponentData.size();
			m_ComponentData.push_back(T());
			m_ActiveStates.Reserve(index);
			m_ActiveStates.Set(index);
			return &m_ComponentData[index];
		}

		virtual void* Create(EntityID entityID, void* data) override
		{
			T& pOther = *static_cast<T*>(data);
			m_Entities.push_back(entityID);
			size_t index = m_ComponentData.size();
			m_ComponentData.push_back(pOther);
			m_ActiveStates.Reserve(index);
			m_ActiveStates.Set(index);
			return &m_ComponentData[index];
		}

		virtual void* GetComponentAddress(EntityID entityID, size_t number = 0) override
		{
			const uint32_t index = GetComponentIndex(entityID, number);
			return &m_ComponentData[index];
		}

		virtual const void* GetComponentAddress(EntityID entityID, size_t number = 0) const override
		{
			const uint32_t index = GetComponentIndex(entityID, number);
			return &m_ComponentData[index];
		}

		virtual const void* GetComponentAddressFromIndex(size_t index) const override
		{
			return &m_ComponentData[index];
		}

		virtual void* GetComponentAddressFromIndex(size_t index) override
		{
			return &m_ComponentData[index];
		}

		void Invoke(const InvocationType& callbackType, EntityRegistry* pRegistry, EntityID entity, void* pComponentAddress) override
		{
			if (!pRegistry->CallbacksEnabled() || !pRegistry->CallbackEnabled(callbackType)) return;
			EntityView* pEntityView = pRegistry->GetEntityView(entity);
			if (!pEntityView) return;
			const uint32_t index = GetComponentIndex(entity);
			switch (callbackType)
			{
			case InvocationType::OnEnableDraw:
			case InvocationType::Update:
			case InvocationType::Draw:
				if (!pEntityView->IsActive() || !m_ActiveStates.IsSet(index)) return;
				break;
			}
			T* pComponent = (T*)pComponentAddress;
			m_Callbacks->Invoke(callbackType, pRegistry, entity, *pComponent);
		}

		void InvokeAll(const InvocationType& invocationType, EntityRegistry* pRegistry, std::function<bool(BaseTypeView*, EntityView*, size_t)> canCallCallback) override
		{
			if (!pRegistry->CallbacksEnabled() || !pRegistry->CallbackEnabled(invocationType)) return;
			size_t count = m_ComponentData.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (!pRegistry->CallbacksEnabled() || !pRegistry->CallbackEnabled(invocationType)) return;
				T& component = m_ComponentData[i];
				EntityID entity = m_Entities[i];
				EntityView* pEntityView = pRegistry->GetEntityView(entity);
				const bool isActive = pEntityView->IsActive() && m_ActiveStates.IsSet(uint32_t(i));
				if (!pEntityView) continue;
				const bool canCall = canCallCallback ? canCallCallback(this, pEntityView, i) : true;
				if (!canCall) continue;
				switch (invocationType)
				{
				case InvocationType::Update:
				case InvocationType::Draw:
					if (!isActive) continue;
					break;
				}
				m_Callbacks->Invoke(invocationType, pRegistry, entity, component);
				if (m_ComponentData.size() < count)
					/* Component was removed during last invoke */
					count = m_ComponentData.size();
			}
		}

		void InvokeAll(const InvocationType& invocationType, EntityRegistry* pRegistry, const std::vector<EntityID>& entities) override
		{
			if (!pRegistry->CallbacksEnabled() || !pRegistry->CallbackEnabled(invocationType)) return;
			for (auto entity : entities)
			{
				if (!pRegistry->CallbacksEnabled() || !pRegistry->CallbackEnabled(invocationType)) return;
				const uint32_t index = GetComponentIndex(entity);
				if (index >= m_Entities.size()) continue;
				Invoke(invocationType, pRegistry, m_Entities[index], &m_ComponentData[index]);
			}
		}

	private:
		virtual BaseTypeView* Create(EntityRegistry* pRegistry) override
		{
			if (!m_IsFactory)
				throw new std::exception("Cannot create TypeView: Not a factory!");

			TypeView<T>* pTypeView = new TypeView<T>(pRegistry);
			pTypeView->m_Callbacks = this->m_Callbacks;
			return pTypeView;
		}

		virtual void GetReferences(std::vector<UUID>& references) const
		{
			m_Callbacks->InvokeReferencesCallback(this, references);
		}

	private:
		friend class EntityRegistry;
		friend class ComponentTypes;
		std::vector<T> m_ComponentData;
		ComponentInvokations<T>* m_Callbacks;
		bool m_IsFactory = false;
	};
}
