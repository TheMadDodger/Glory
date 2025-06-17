#include "EntityRegistry.h"
#include "ComponentTypes.h"

namespace Glory::Utils::ECS
{
	EntityRegistry::EntityRegistry() : m_EntityDirty(32, true),
		m_EnabledCallbacks(size_t(InvocationType::Count), true),
		m_NextEntityID(1), m_pUserData(nullptr)
	{
	}

	EntityRegistry::EntityRegistry(void* pUserData) : m_EntityDirty(32, true),
		m_EnabledCallbacks(size_t(InvocationType::Count), true),
		m_NextEntityID(1), m_pUserData(pUserData) {}

	EntityRegistry::~EntityRegistry()
	{
		Reset();
	}

	EntityID EntityRegistry::CreateEntity()
	{
		EntityID newEntity = m_NextEntityID;
		++m_NextEntityID;
		m_pEntityViews.emplace(newEntity, new EntityView(this));
		m_RootOrder.push_back(newEntity);
		m_EntityDirty.Reserve(newEntity);
		m_EntityDirty.Set(newEntity);
		return newEntity;
	}

	void EntityRegistry::DestroyEntity(EntityID entity)
	{
		EntityView* pEntityView = GetEntityView(entity);

		/* Remove all components */
		for (auto it = pEntityView->m_ComponentTypes.begin(); it != pEntityView->m_ComponentTypes.end(); it++)
		{
			uint32_t typeHash = it->second;
			BaseTypeView* pTypeView = GetTypeView(typeHash);
			void* pAddress = pTypeView->GetComponentAddress(entity);
			pTypeView->Invoke(InvocationType::OnDisable, this, entity, pAddress);
			pTypeView->Invoke(InvocationType::Stop, this, entity, pAddress);
			pTypeView->Invoke(InvocationType::OnRemove, this, entity, pAddress);
			pTypeView->Remove(entity);
		}

		/* Update parent */
		if (pEntityView->m_Parent)
		{
			/* Remove from parent children array */
			EntityView* pParentView = m_pEntityViews.at(pEntityView->m_Parent);
			const auto itor = std::find(pParentView->m_Children.begin(), pParentView->m_Children.end(), entity);
			pParentView->m_Children.erase(itor);
			SetEntityDirty(pEntityView->m_Parent);
		}
		else
		{
			/* Remove from root order */
			const auto itor = std::find(m_RootOrder.begin(), m_RootOrder.end(), entity);
			m_RootOrder.erase(itor);
		}

		/* Delete view */
		delete pEntityView;
		m_pEntityViews.erase(entity);
	}

	void* EntityRegistry::CreateComponent(EntityID entityID, uint32_t typeHash, Glory::UUID uuid)
	{
		BaseTypeView* pTypeView = GetTypeView(typeHash);
		const ComponentType* componentType = ComponentTypes::GetComponentType(pTypeView->m_TypeHash);
		if (!componentType->m_AllowMultiple && pTypeView->Contains(entityID))
		{
			throw new std::exception(("Duplicate component of type " + componentType->m_Name + " not allowed!").c_str());
		}

		void* pAddress = pTypeView->Create(entityID);
		EntityView* pEntityView = GetEntityView(entityID);
		pEntityView->Add(pTypeView->m_TypeHash, uuid);
		pTypeView->Invoke(InvocationType::OnAdd, this, entityID, pAddress);
		SetEntityDirty(entityID);
		return pAddress;
	}

	void* EntityRegistry::CopyComponent(EntityID entityID, uint32_t typeHash, Glory::UUID uuid, void* data)
	{
		BaseTypeView* pTypeView = GetTypeView(typeHash);
		const ComponentType* componentType = ComponentTypes::GetComponentType(pTypeView->m_TypeHash);
		if (!componentType->m_AllowMultiple && pTypeView->Contains(entityID))
		{
			throw new std::exception(("Duplicate component of type " + componentType->m_Name + " not allowed!").c_str());
		}

		void* pAddress = pTypeView->Create(entityID, data);
		EntityView* pEntityView = GetEntityView(entityID);
		pEntityView->Add(pTypeView->m_TypeHash, uuid);
		pTypeView->Invoke(InvocationType::OnAdd, this, entityID, pAddress);
		pTypeView->Invoke(InvocationType::OnValidate, this, entityID, pAddress);
		SetEntityDirty(entityID);
		return pAddress;
	}

	BaseTypeView* EntityRegistry::GetTypeView(uint32_t typeHash)
	{
		if (m_ViewIndices.find(typeHash) == m_ViewIndices.end())
			return ComponentTypes::CreateTypeView(this, typeHash);

		const size_t index = m_ViewIndices.at(typeHash);
		return m_pViews[index];
	}

	EntityView* EntityRegistry::GetEntityView(EntityID entity)
	{
		const auto itor = m_pEntityViews.find(entity);
		if (itor == m_pEntityViews.end())
			return nullptr;

		return itor->second;
	}

	EntityView* EntityRegistry::GetEntityView(EntityID entity) const
	{
		const auto itor = m_pEntityViews.find(entity);
		if (itor == m_pEntityViews.end())
			return nullptr;

		return itor->second;
	}

	std::map<EntityID, EntityView*>::const_iterator EntityRegistry::EntityViewBegin() const
	{
		return m_pEntityViews.begin();
	}

	std::map<EntityID, EntityView*>::const_iterator EntityRegistry::EntityViewEnd() const
	{
		return m_pEntityViews.end();
	}

	void* EntityRegistry::GetComponentAddress(EntityID entityID, Glory::UUID componentID)
	{
		EntityView* pEntityView = GetEntityView(entityID);
		uint32_t hash = pEntityView->m_ComponentTypes.at(componentID);
		BaseTypeView* pTypeView = GetTypeView(hash);

		// TODO: Get number of the component in case of duplicates
		return pTypeView->GetComponentAddress(entityID);
	}

	bool EntityRegistry::HasComponent(EntityID entity, uint32_t type)
	{
		BaseTypeView* pTypeView = GetTypeView(type);
		return pTypeView->Contains(entity);
	}

	UUID EntityRegistry::RemoveComponent(EntityID entity, uint32_t typeHash)
	{
		EntityView* pEntityView = GetEntityView(entity);
		BaseTypeView* pTypeView = GetTypeView(typeHash);
		void* pAddress = pTypeView->GetComponentAddress(entity);
		pTypeView->Remove(entity);
		SetEntityDirty(entity);
		return m_pEntityViews[entity]->Remove(typeHash);
	}

	void EntityRegistry::RemoveComponentAt(EntityID entity, size_t index)
	{
		EntityView* pEntityView = GetEntityView(entity);
		uint32_t typeHash = pEntityView->ComponentTypeAt(index);
		BaseTypeView* pTypeView = GetTypeView(typeHash);
		void* pAddress = pTypeView->GetComponentAddress(entity);
		pTypeView->Remove(entity);
		m_pEntityViews[entity]->Remove(typeHash);
		SetEntityDirty(entity);
	}

	size_t EntityRegistry::ComponentCount(EntityID entity)
	{
		EntityView* pEntityView = GetEntityView(entity);
		return pEntityView->ComponentCount();
	}

	void EntityRegistry::Clear(EntityID entity)
	{
		EntityView* pEntityView = GetEntityView(entity);
		for (auto it = pEntityView->m_ComponentTypes.begin(); it != pEntityView->m_ComponentTypes.end(); it++)
		{
			uint32_t typeHash = it->second;
			BaseTypeView* pTypeView = GetTypeView(typeHash);
			void* pAddress = pTypeView->GetComponentAddress(entity);
			pTypeView->Remove(entity);
			SetEntityDirty(entity);
		}
	}

	const size_t EntityRegistry::Alive() const
	{
		return m_pEntityViews.size();
	}

	const bool EntityRegistry::IsValid(EntityID entity) const
	{
		return m_pEntityViews.find(entity) != m_pEntityViews.end();
	}

	const size_t EntityRegistry::TypeViewCount() const
	{
		return m_pViews.size();
	}

	BaseTypeView* EntityRegistry::TypeViewAt(size_t index) const
	{
		return m_pViews[index];
	}

	void EntityRegistry::InvokeAll(uint32_t typeHash, InvocationType invocationType)
	{
		if (!m_CallbacksEnabled || !CallbackEnabled(invocationType)) return;
		BaseTypeView* pTypeView = GetTypeView(typeHash);
		pTypeView->InvokeAll(invocationType, this, NULL);
	}

	void EntityRegistry::SetUserData(void* data)
	{
		m_pUserData = data;
	}

	void EntityRegistry::ForEach(std::function<void(EntityRegistry*, EntityID)> func)
	{
		for (auto& itor : m_pEntityViews)
		{
			func(this, itor.first);
		}
	}

	EntityID EntityRegistry::GetParent(Utils::ECS::EntityID entity) const
	{
		const auto itor = m_pEntityViews.find(entity);
		if (itor == m_pEntityViews.end()) return false;
		return itor->second->Parent();
	}

	bool EntityRegistry::SetParent(Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const auto itor1 = m_pEntityViews.find(entity);

		if (itor1 == m_pEntityViews.end()) return false;
		const Utils::ECS::EntityID oldParent = itor1->second->m_Parent;
		if (oldParent == parent) return true;
		if (oldParent)
		{
			auto& oldParentChildren = m_pEntityViews.at(oldParent)->m_Children;
			const auto itor = std::find(oldParentChildren.begin(), oldParentChildren.end(), entity);
			oldParentChildren.erase(itor);
			SetEntityDirty(oldParent);
		}
		else
		{
			const auto itor = std::find(m_RootOrder.begin(), m_RootOrder.end(), entity);
			m_RootOrder.erase(itor);
		}

		itor1->second->m_Parent = parent;
		const auto itor2 = m_pEntityViews.find(parent);
		if (itor2 != m_pEntityViews.end())
		{
			itor2->second->m_Children.push_back(entity);
		}
		else
		{
			m_RootOrder.push_back(entity);
		}

		return true;
	}

	size_t EntityRegistry::ChildCount(Utils::ECS::EntityID entity) const
	{
		if (entity == 0)
			return m_RootOrder.size();

		const auto itor = m_pEntityViews.find(entity);
		return itor->second->ChildCount();
	}

	EntityID EntityRegistry::Child(Utils::ECS::EntityID entity, size_t index) const
	{
		if (entity == 0)
			return m_RootOrder[index];

		const auto itor = m_pEntityViews.find(entity);
		return itor->second->Child(index);
	}

	size_t EntityRegistry::SiblingIndex(Utils::ECS::EntityID entity) const
	{
		auto itor = m_pEntityViews.find(entity);
		itor = m_pEntityViews.find(itor->second->Parent());
		if (itor == m_pEntityViews.end())
		{
			const auto rootItor = std::find(m_RootOrder.begin(), m_RootOrder.end(), entity);
			if (rootItor == m_RootOrder.end()) return 0;
			return rootItor - m_RootOrder.begin();
		}
		return itor->second->ChildIndex(entity);
	}

	void EntityRegistry::SetSiblingIndex(Utils::ECS::EntityID entity, size_t index)
	{
		auto itor = m_pEntityViews.find(entity);
		const Utils::ECS::EntityID parent = itor->second->Parent();
		itor = m_pEntityViews.find(parent);
		std::vector<EntityID>* targetVector = nullptr;
		if (itor == m_pEntityViews.end())
		{
			targetVector = &m_RootOrder;
			SetEntityDirty(entity);
		}
		else
		{
			targetVector = &itor->second->m_Children;
			SetEntityDirty(parent);
		}
		auto it = std::find(targetVector->begin(), targetVector->end(), entity);
		const size_t oldIndex = it - targetVector->begin();
		if (index > 0 && oldIndex < index)
			index -= 1;

		if (it == targetVector->end()) return;
		targetVector->erase(it);
		if (index >= targetVector->size())
		{
			targetVector->push_back(entity);
			return;
		}
		targetVector->insert(targetVector->begin() + index, entity);
	}

	void EntityRegistry::ResizeRootOrder(size_t size)
	{
		m_RootOrder.resize(size);
	}

	std::vector<EntityID>& EntityRegistry::RootOrder()
	{
		return m_RootOrder;
	}

	const std::vector<EntityID>& EntityRegistry::RootOrder() const
	{
		return m_RootOrder;
	}

	EntityID EntityRegistry::CopyEntityToOtherRegistry(EntityID entity, EntityID parent, EntityRegistry* pRegistry)
	{
		const EntityID newEntity = pRegistry->CreateEntity();
		EntityView* pNewEntityView = pRegistry->GetEntityView(newEntity);

		if (parent) pRegistry->SetParent(newEntity, parent);
		EntityView* pEntityView = GetEntityView(entity);
		pNewEntityView->Active() = pEntityView->Active();
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			const uint32_t type = pEntityView->ComponentTypeAt(i);
			const UUID uuid = pEntityView->ComponentUUIDAt(i);
			void* data = GetComponentAddress(entity, uuid);
			pRegistry->CopyComponent(newEntity, type, uuid, data);
		}

		return newEntity;
	}

	bool EntityRegistry::IsEntityDirty(EntityID entity) const
	{
		return m_EntityDirty.IsSet(entity);
	}

	void EntityRegistry::SetEntityDirty(EntityID entity, bool dirty, bool setChildrenDirty)
	{
		EntityView* pView = GetEntityView(entity);
		if (!pView) return;

		if (dirty == IsEntityDirty(entity)) return;
		m_EntityDirty.Set(entity, dirty);
		
		if (!dirty) return;

		InvokeAll(InvocationType::OnDirty, { entity });
		/* Notify parent once */
		const Utils::ECS::EntityID parent = GetParent(entity);
		if (parent && !IsEntityDirty(parent))
		{
			m_EntityDirty.Set(parent, dirty);
			InvokeAll(InvocationType::OnDirty, { parent });
		}

		if (!setChildrenDirty) return;
		/* Must set all children as dirty as well! */
		for (size_t i = 0; i < pView->ChildCount(); ++i)
		{
			EntityID child = pView->Child(i);
			SetEntityDirty(child, dirty);
		}
	}

	void EntityRegistry::InvokeAll(InvocationType invocationType, std::function<bool(BaseTypeView*, EntityView*, size_t)> canCallCallback)
	{
		if (!m_CallbacksEnabled || !CallbackEnabled(invocationType)) return;
		for (size_t i = 0; i < m_pViews.size(); ++i)
		{
			if (!m_CallbacksEnabled) return;
			m_pViews[i]->InvokeAll(invocationType, this, canCallCallback);
		}
	}

	void EntityRegistry::InvokeAll(InvocationType invocationType, const std::vector<EntityID>& entities)
	{
		if (!m_CallbacksEnabled || !CallbackEnabled(invocationType)) return;
		for (size_t i = 0; i < m_pViews.size(); ++i)
		{
			if (!m_CallbacksEnabled) return;
			m_pViews[i]->InvokeAll(invocationType, this, entities);
		}
	}

	void EntityRegistry::DisableCallbacks()
	{
		m_CallbacksEnabled = false;
	}

	bool EntityRegistry::CallbacksEnabled() const
	{
		return m_CallbacksEnabled;
	}

	bool EntityRegistry::CallbackEnabled(InvocationType type) const
	{
		return m_EnabledCallbacks.IsSet(uint32_t(type));
	}

	void EntityRegistry::EnableAllIndividualCallbacks()
	{
		m_EnabledCallbacks.SetAll();
	}

	void EntityRegistry::SetCallbackEnabled(InvocationType type, bool enabled)
	{
		m_EnabledCallbacks.Set(uint32_t(type), enabled);
	}

	void EntityRegistry::Reset()
	{
		if (m_CallbacksEnabled)
			InvokeAll(InvocationType::OnRemove, NULL);

		for (size_t i = 0; i < m_pViews.size(); ++i)
		{
			delete m_pViews[i];
		}

		for (auto it = m_pEntityViews.begin(); it != m_pEntityViews.end(); it++)
		{
			delete it->second;
		}

		m_pEntityViews.clear();
		m_pViews.clear();
		m_ViewIndices.clear();
		m_RootOrder.clear();

		m_pUserData = nullptr;
		m_NextEntityID = 1;
	}

	EntityID EntityRegistry::MaxEntityID() const
	{
		return m_NextEntityID;
	}
}
