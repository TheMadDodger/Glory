#include "BinarySerialization.h"
#include "BinaryStream.h"

#include <Reflection.h>
#include <EntityID.h>
#include <EntityRegistry.h>
#include <BinaryBuffer.h>

namespace Glory
{
	void SerializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data)
	{
		const uint32_t type = pFieldData->Type();
		const uint32_t elementType = pFieldData->ArrayElementType();
		const Utils::Reflect::TypeData* pElementTypeData = Utils::Reflect::Reflect::GetTyeData(elementType);

		switch (type)
		{
		case uint32_t(CustomTypeHash::Struct): {
			for (size_t i = 0; i < pElementTypeData->FieldCount(); ++i)
			{
				const Utils::Reflect::FieldData* pField = pElementTypeData->GetFieldData(i);
				void* pAddress = pField->GetAddress(data);
				SerializeData(container, pField, pAddress);
			}
			break;
		}

		case uint32_t(CustomTypeHash::Array): {
			const size_t arraySize = Utils::Reflect::Reflect::ArraySize(data, elementType);
			container.Write(arraySize);
			for (size_t i = 0; i < arraySize; ++i)
			{
				const Utils::Reflect::FieldData* pElementField = pFieldData->GetArrayElementFieldData(i);
				void* pAddress = pElementField->GetAddress(data);
				SerializeData(container, pElementField, pAddress);
			}
			break;
		}
		case uint32_t(CustomTypeHash::Buffer): {
			BinaryBuffer* buffer = (BinaryBuffer*)data;
			container.Write(buffer->m_Buffer);
			break;
		}

		case uint32_t(CustomTypeHash::Enum):
		case uint32_t(CustomTypeHash::Basic):
		default: {
			const size_t size = pFieldData->Size();
			container.Write(data, size);
			break;
		}
		}
	}

	void SerializeTree(BinaryStream& container, const Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID parent)
	{
		/* Child count first */
		container.Write(parent);

		if (parent == 0) {
			/* We only need to serialize the order */
			const auto& rootOrder = registry.RootOrder();
			container.Write(rootOrder.size()).Write(rootOrder.data(), sizeof(Utils::ECS::EntityID) * rootOrder.size());
			return;
		}

		/* Child order */
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(parent);
		const auto& childOrder = pEntityView->ChildOrder();
		container.Write(childOrder.size()).Write(childOrder.data(), sizeof(Utils::ECS::EntityID) * childOrder.size());

		/* Parent of this entity */
		container.Write(pEntityView->Parent()).Write(pEntityView->HierarchyActive());

		/* Component order */
		const auto& componentOrder = pEntityView->ComponentsOrder();
		container.Write(componentOrder.size()).Write(componentOrder.data(), sizeof(UUID) * componentOrder.size());

		/* Component types */
		for (size_t i = 0; i < componentOrder.size(); ++i)
		{
			const UUID uuid = componentOrder[i];
			const uint32_t type = pEntityView->ComponentType(uuid);
			container.Write(type);
		}
	}

	void DeserializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data)
	{
		const uint32_t type = pFieldData->Type();
		const uint32_t elementType = pFieldData->ArrayElementType();
		const Utils::Reflect::TypeData* pElementTypeData = Utils::Reflect::Reflect::GetTyeData(elementType);

		switch (type)
		{
		case uint32_t(CustomTypeHash::Struct): {
			for (size_t i = 0; i < pElementTypeData->FieldCount(); ++i)
			{
				const Utils::Reflect::FieldData* pField = pElementTypeData->GetFieldData(i);
				void* pAddress = pField->GetAddress(data);
				DeserializeData(container, pField, pAddress);
			}
			break;
		}

		case uint32_t(CustomTypeHash::Array): {
			size_t arraySize;
			container.Read(arraySize);
			Utils::Reflect::Reflect::ResizeArray(data, elementType, arraySize);
			for (size_t i = 0; i < arraySize; ++i)
			{
				const Utils::Reflect::FieldData* pElementField = pFieldData->GetArrayElementFieldData(i);
				void* pAddress = pElementField->GetAddress(data);
				DeserializeData(container, pElementField, pAddress);
			}
			break;
		}
		case uint32_t(CustomTypeHash::Buffer): {
			BinaryBuffer* buffer = (BinaryBuffer*)data;
			size_t size;
			container.Read(size);
			if (size) container.Read(buffer->m_Buffer, size);
			break;
		}

		case uint32_t(CustomTypeHash::Enum):
		case uint32_t(CustomTypeHash::Basic):
		default: {
			const size_t size = pFieldData->Size();
			container.Read(data, size);
			break;
		}
		}
	}

	void DeserializeTree(BinaryStream& container, Utils::ECS::EntityRegistry& registry)
	{
		Utils::ECS::EntityID parent;
		container.Read(parent);

		/* Child count first */
		size_t childCount;
		container.Read(childCount);

		if (parent == 0) {
			/* We only need to deserialize the order */
			registry.ResizeRootOrder(childCount);
			container.Read(registry.RootOrder().data(), sizeof(Utils::ECS::EntityID) * childCount);
			return;
		}

		/* Child order */
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(parent);
		pEntityView->ResizeChildren(childCount);
		container.Read(pEntityView->ChildOrder().data(), sizeof(Utils::ECS::EntityID) * childCount);

		/* Parent of this entity */
		Utils::ECS::EntityID myParent;
		container.Read(myParent).Read(pEntityView->HierarchyActive());
		pEntityView->SetParent(myParent);

		/* Component order */
		size_t componentCount;
		container.Read(componentCount);
		pEntityView->ResizeComponentsOrder(componentCount);
		container.Read(pEntityView->ComponentsOrder().data(), sizeof(UUID) * componentCount);

		/* Component types */
		for (size_t i = 0; i < componentCount; ++i)
		{
			const UUID uuid = pEntityView->ComponentUUIDAt(i);
			uint32_t type;
			container.Read(type);
			pEntityView->SetType(uuid, type);
		}
	}
}