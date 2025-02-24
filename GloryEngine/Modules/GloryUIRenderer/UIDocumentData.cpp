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

	Utils::ECS::EntityID UIDocumentData::CreateEntity(std::string_view name)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity<UITransform>();
		m_Names.emplace(entity, name);
		return entity;
	}

	void UIDocumentData::Serialize(BinaryStream& container) const
	{
	}

	void UIDocumentData::Deserialize(BinaryStream& container)
	{
	}
}
