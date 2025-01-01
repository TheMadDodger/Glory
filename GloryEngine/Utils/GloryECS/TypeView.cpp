#include "TypeView.h"

namespace Glory::Utils::ECS
{
	BaseTypeView::BaseTypeView(uint32_t typeHash, EntityRegistry* pRegistry)
		: m_TypeHash(typeHash), m_pRegistry(pRegistry), m_Entities(),
		m_ActiveStates(32, true)
	{
	}

	BaseTypeView::~BaseTypeView()
	{
		m_Entities.clear();
	}

	void BaseTypeView::Remove(EntityID entityID)
	{
		auto it = std::find(m_Entities.begin(), m_Entities.end(), entityID);
		if (it == m_Entities.end()) return;
		size_t index = it - m_Entities.begin();
		m_Entities.erase(it);
		OnRemove(index);
	}

	bool BaseTypeView::Contains(EntityID entityID)
	{
		return std::find(m_Entities.begin(), m_Entities.end(), entityID) != m_Entities.end();
	}

	const uint32_t BaseTypeView::ComponentTypeHash() const
	{
		return m_TypeHash;
	}

	uint32_t BaseTypeView::GetComponentIndex(EntityID entityID, size_t number) const
	{
		auto it = std::find_if(m_Entities.begin(), m_Entities.end(), [&](EntityID othereEntity)
		{
			if (othereEntity != entityID) return false;
			if (number != 0)
			{
				--number;
				return false;
			}
			return true;
		});
		return uint32_t(it - m_Entities.begin());
	}

	bool BaseTypeView::IsActive(EntityID entity) const
	{
		const uint32_t index = GetComponentIndex(entity, 0);
		return m_ActiveStates.IsSet(index);
	}

	bool BaseTypeView::IsActiveByIndex(size_t index) const
	{
		return m_ActiveStates.IsSet(index);
	}

	void BaseTypeView::SetActive(EntityID entity, bool active)
	{
		const uint32_t index = GetComponentIndex(entity, 0);
		const bool wasActive = m_ActiveStates.IsSet(index);
		m_ActiveStates.Set(index, active);
	}

	void BaseTypeView::SetActiveByIndex(size_t index, bool active)
	{
		const bool wasActive = m_ActiveStates.IsSet(index);
		m_ActiveStates.Set(index, active);
	}

	size_t BaseTypeView::Size() const
	{
		return m_Entities.size();
	}

	EntityID BaseTypeView::EntityAt(size_t index) const
	{
		return m_Entities[index];
	}

	BitSet& BaseTypeView::ActiveStates()
	{
		return m_ActiveStates;
	}

	const BitSet& BaseTypeView::ActiveStates() const
	{
		return m_ActiveStates;
	}
}
