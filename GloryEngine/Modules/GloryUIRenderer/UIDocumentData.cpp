#include "UIDocumentData.h"
#include "UIComponents.h"

namespace Glory
{
	UIDocumentData::UIDocumentData()
	{
		APPEND_TYPE(UIDocumentData);
	}

	UIDocumentData::~UIDocumentData()
	{
	}

	Utils::ECS::EntityRegistry& UIDocumentData::GetRegistry()
	{
		return m_Registry;
	}

	const std::string& UIDocumentData::Name(Utils::ECS::EntityID entity) const
	{
		return m_Names.at(entity);
	}

	Utils::ECS::EntityID UIDocumentData::CreateEmptyEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	Utils::ECS::EntityID UIDocumentData::CreateEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity<UITransform>();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	UUID UIDocumentData::EntityUUID(Utils::ECS::EntityID entity) const
	{
		return m_UUIds.at(entity);
	}

	Utils::ECS::EntityID UIDocumentData::EntityID(UUID uuid) const
	{
		return m_Ids.at(uuid);
	}

	void UIDocumentData::Reset()
	{
		m_Registry.Reset();
		m_Ids.clear();
		m_UUIds.clear();
		m_Names.clear();
	}

	void UIDocumentData::Serialize(BinaryStream& container) const
	{
	}

	void UIDocumentData::Deserialize(BinaryStream& container)
	{
	}
}
