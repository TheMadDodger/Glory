#pragma once

namespace Glory
{
	class BinaryStream;
	namespace Utils::Reflect
	{
		struct FieldData;
	}

	void SerializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
	void SerializeTree(BinaryStream& container, const Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID parent);
	void DeserializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
	void DeserializeTree(BinaryStream& container, Utils::ECS::EntityRegistry& registry);
}