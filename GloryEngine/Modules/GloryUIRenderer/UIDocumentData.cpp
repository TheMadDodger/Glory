#include "UIDocumentData.h"
#include "UIComponents.h"

#include <BinaryStream.h>
#include <BinarySerialization.h>

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
		const size_t entityCount = m_Registry.Alive();
		const size_t typeViewCount = m_Registry.TypeViewCount();
		container.Write(entityCount).Write(typeViewCount);

		/* Serialize component datas */
		for (size_t i = 0; i < typeViewCount; ++i)
		{
			Utils::ECS::BaseTypeView* pTypeView = m_Registry.TypeViewAt(i);
			const uint32_t hash = pTypeView->ComponentTypeHash();
			container.Write(hash);
			container.Write(pTypeView->Size());

			const Utils::Reflect::TypeData* pTypeData =
				Utils::Reflect::Reflect::GetTyeData(hash);

			const int bufferOffset = pTypeData->DataBufferOffset();
			const size_t bufferSize = pTypeData->DataBufferSize();
			for (size_t j = 0; j < pTypeView->Size(); ++j)
			{
				Utils::ECS::EntityID entity = pTypeView->EntityAt(j);
				container.Write(entity);
				void* data = pTypeView->GetComponentAddress(entity);
				for (size_t k = 0; k < pTypeData->FieldCount(); ++k)
				{
					const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(k);
					void* pAddress = pField->GetAddress(data);
					SerializeData(container, pField, pAddress);
				}
				if (bufferOffset != -1)
				{
					const char* pBuffer = (const char*)data + bufferOffset;
					container.Write(pBuffer, bufferSize);
				}
			}
			container.Write(pTypeView->ActiveStates());
		}

		/* Serialize the hierarchy */
		SerializeTree(container, m_Registry, 0);
		for (auto itor = m_Registry.EntityViewBegin(); itor != m_Registry.EntityViewEnd(); ++itor)
		{
			const Utils::ECS::EntityID entity = itor->first;
			SerializeTree(container, m_Registry, entity);
		}

		/* Serialize scene IDs and names */
		container.Write(m_Ids.size());
		for (auto itor = m_Ids.begin(); itor != m_Ids.end(); ++itor)
		{
			container.Write(itor->first).Write(itor->second).Write(m_Names.at(itor->second));
		}
	}

	void UIDocumentData::Deserialize(BinaryStream& container)
	{
		size_t typeViewCount, entityCount;
		container.Read(entityCount).Read(typeViewCount);

		for (size_t i = 0; i < entityCount; ++i)
			m_Registry.CreateEntity();

		/* Deserialize component datas */
		for (size_t i = 0; i < typeViewCount; ++i)
		{
			uint32_t hash;
			container.Read(hash);
			Utils::ECS::BaseTypeView* pTypeView = m_Registry.GetTypeView(hash);

			size_t size;
			container.Read(size);

			const Utils::Reflect::TypeData* pTypeData =
				Utils::Reflect::Reflect::GetTyeData(hash);

			const int bufferOffset = pTypeData->DataBufferOffset();
			const size_t bufferSize = pTypeData->DataBufferSize();

			for (size_t j = 0; j < size; ++j)
			{
				Utils::ECS::EntityID entity;
				container.Read(entity);
				void* data = pTypeView->Create(entity);
				for (size_t k = 0; k < pTypeData->FieldCount(); ++k)
				{
					const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(k);
					void* pAddress = pField->GetAddress(data);
					DeserializeData(container, pField, pAddress);
				}

				if (bufferOffset != -1)
				{
					char* pBuffer = (char*)data + bufferOffset;
					container.Read(pBuffer, bufferSize);
				}
			}
			container.Read(pTypeView->ActiveStates());
		}

		/* Deserialize the hierarchy */
		for (size_t i = 0; i < entityCount + 1; ++i)
		{
			DeserializeTree(container, m_Registry);
		}

		/* Deserialize scene IDs and names */
		size_t idSize;
		container.Read(idSize);
		for (size_t i = 0; i < idSize; i++)
		{
			UUID id;
			Utils::ECS::EntityID entity;
			std::string name;
			container.Read(id).Read(entity).Read(name);
			m_Ids.emplace(id, entity);
			m_UUIds.emplace(entity, id);
			m_Names.emplace(entity, name);
		}
	}

	void UIDocumentData::References(Engine* pEngine, std::vector<UUID>& references) const
	{
		const size_t typeViewCount = m_Registry.TypeViewCount();
		for (size_t i = 0; i < typeViewCount; ++i)
		{
			Utils::ECS::BaseTypeView* pTypeView = m_Registry.TypeViewAt(i);
			pTypeView->GetReferences(references);
		}
	}
}
