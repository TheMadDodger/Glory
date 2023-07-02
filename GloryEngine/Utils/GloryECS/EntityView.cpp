#include "EntityView.h"
#include "EntityRegistry.h"

namespace GloryECS
{
    EntityView::EntityView(EntityRegistry* pRegistry) : m_pRegistry(pRegistry), m_Active(true), m_HierarchyActive(true)
    {
    }

    size_t EntityView::ComponentCount()
    {
        return m_ComponentOrder.size();
    }

    Glory::UUID EntityView::ComponentUUIDAt(size_t index)
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

    bool EntityView::IsActive()
    {
        return m_HierarchyActive && m_Active;
    }

    uint32_t EntityView::ComponentTypeAt(size_t index)
    {
        Glory::UUID uuid = m_ComponentOrder[index];
        return ComponentType(uuid);
    }

    uint32_t EntityView::ComponentType(Glory::UUID uuid)
    {
        if (m_ComponentTypes.find(uuid) == m_ComponentTypes.end()) return 0;
        return m_ComponentTypes[uuid];
    }

    void EntityView::Add(uint32_t hash, Glory::UUID uuid)
    {
        m_TypeToUUID.emplace(hash, uuid);
        m_ComponentTypes.emplace(uuid, hash);
        m_ComponentOrder.push_back(uuid);
    }

    void EntityView::Remove(uint32_t hash)
    {
        if (m_TypeToUUID.find(hash) == m_TypeToUUID.end()) return;
        Glory::UUID uuid = m_TypeToUUID[hash];
        m_TypeToUUID.erase(hash);
        m_ComponentTypes.erase(uuid);
        auto it = std::find(m_ComponentOrder.begin(), m_ComponentOrder.end(), uuid);
        m_ComponentOrder.erase(it);
    }

    void EntityView::Remove(Glory::UUID uuid)
    {
        if (m_ComponentTypes.find(uuid) == m_ComponentTypes.end()) return;
        uint32_t hash = m_ComponentTypes[uuid];
        m_ComponentTypes.erase(uuid);
        m_TypeToUUID.erase(hash);
        auto it = std::find(m_ComponentOrder.begin(), m_ComponentOrder.end(), uuid);
        m_ComponentOrder.erase(it);
    }
}
