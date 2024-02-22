#include "EntityView.h"
#include "EntityRegistry.h"

namespace Glory::Utils::ECS
{
    EntityView::EntityView(EntityRegistry* pRegistry)
        : m_pRegistry(pRegistry), m_Active(true), m_HierarchyActive(true), m_Parent(0)
    {}

    size_t EntityView::ComponentCount() const
    {
        return m_ComponentOrder.size();
    }

    Glory::UUID EntityView::ComponentUUIDAt(size_t index) const
    {
        return m_ComponentOrder[index];
    }

    std::map<Glory::UUID, uint32_t>::iterator EntityView::GetIterator()
    {
        return m_ComponentTypes.begin();
    }

    std::map<Glory::UUID, uint32_t>::iterator EntityView::GetIteratorEnd()
    {
        return m_ComponentTypes.end();
    }

    void EntityView::SwapComponentIndex(size_t index1, size_t index2)
    {
        if (index1 >= m_ComponentOrder.size() || index2 >= m_ComponentOrder.size()) return;
        Glory::UUID comp1 = m_ComponentOrder[index1];
        Glory::UUID comp2 = m_ComponentOrder[index2];
        m_ComponentOrder[index1] = comp2;
        m_ComponentOrder[index2] = comp1;
    }

    void EntityView::SetComponentIndex(size_t oldIndex, size_t newIndex)
    {
        if (oldIndex == newIndex) return;
        if (oldIndex >= m_ComponentOrder.size() || newIndex >= m_ComponentOrder.size()) return;
        Glory::UUID uuid = m_ComponentOrder[oldIndex];
        auto eraseIter = m_ComponentOrder.begin() + oldIndex;
        m_ComponentOrder.erase(eraseIter);
        auto insertIter = m_ComponentOrder.begin() + newIndex;
        m_ComponentOrder.insert(insertIter, uuid);
    }

    bool& EntityView::Active()
    {
        return m_Active;
    }

    bool& EntityView::HierarchyActive()
    {
        return m_HierarchyActive;
    }

    bool EntityView::IsActive() const
    {
        return m_HierarchyActive && m_Active;
    }

    EntityID EntityView::Parent() const
    {
        return m_Parent;
    }

    size_t EntityView::ChildCount() const
    {
        return m_Children.size();
    }

    EntityID EntityView::Child(size_t index) const
    {
        return m_Children[index];
    }

    size_t EntityView::ChildIndex(EntityID entity) const
    {
        auto itor = std::find(m_Children.begin(), m_Children.end(), entity);
        if (itor == m_Children.end()) return 0;
        return itor - m_Children.begin();
    }

    void EntityView::ResizeChildren(size_t size)
    {
        m_Children.resize(size);
    }

    std::vector<EntityID>& EntityView::ChildOrder()
    {
        return m_Children;
    }

    void EntityView::SetParent(EntityID parent)
    {
        m_Parent = parent;
    }

    void EntityView::ResizeComponentsOrder(size_t size)
    {
        m_ComponentOrder.resize(size);
    }

    std::vector<Glory::UUID>& EntityView::ComponentsOrder()
    {
        return m_ComponentOrder;
    }

    void EntityView::SetType(Glory::UUID uuid, uint32_t type)
    {
        m_ComponentTypes.emplace(uuid, type);
        m_TypeToUUID.emplace(type, uuid);
    }

    uint32_t EntityView::ComponentTypeAt(size_t index) const
    {
        const Glory::UUID uuid = m_ComponentOrder[index];
        return ComponentType(uuid);
    }

    uint32_t EntityView::ComponentType(Glory::UUID uuid) const
    {
        auto itor = m_ComponentTypes.find(uuid);
        if (itor == m_ComponentTypes.end()) return 0;
        return itor->second;
    }

    void EntityView::Add(uint32_t hash, Glory::UUID uuid)
    {
        m_TypeToUUID.emplace(hash, uuid);
        m_ComponentTypes.emplace(uuid, hash);
        m_ComponentOrder.push_back(uuid);
    }

    UUID EntityView::Remove(uint32_t hash)
    {
        auto itor = m_TypeToUUID.find(hash);
        if (itor == m_TypeToUUID.end()) return 0;
        Glory::UUID uuid = itor->second;
        m_TypeToUUID.erase(hash);
        m_ComponentTypes.erase(uuid);
        auto it = std::find(m_ComponentOrder.begin(), m_ComponentOrder.end(), uuid);
        const uint32_t index = uint32_t(it - m_ComponentOrder.begin());
        m_ComponentOrder.erase(it);
        return uuid;
    }

    void EntityView::Remove(Glory::UUID uuid)
    {
        auto itor = m_ComponentTypes.find(uuid);
        if (itor == m_ComponentTypes.end()) return;
        uint32_t hash = itor->second;
        m_ComponentTypes.erase(uuid);
        m_TypeToUUID.erase(hash);
        auto it = std::find(m_ComponentOrder.begin(), m_ComponentOrder.end(), uuid);
        const uint32_t index = uint32_t(it - m_ComponentOrder.begin());
        m_ComponentOrder.erase(it);
    }
}
