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

	const size_t BaseTypeView::ComponentTypeHash() const
	{
		return m_TypeHash;
	}

	bool BaseTypeView::IsActive(EntityID entity) const
	{
		const uint32_t index = GetComponentIndex(entity, 0);
		return m_ActiveStates.IsSet(index);
	}

	void BaseTypeView::SetActive(EntityID entity, bool active)
	{
		const uint32_t index = GetComponentIndex(entity, 0);
		m_ActiveStates.Set(index, active);
	}
}
