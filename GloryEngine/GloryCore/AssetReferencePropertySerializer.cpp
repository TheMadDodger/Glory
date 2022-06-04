#include "AssetReferencePropertySerializer.h"
#include "AssetManager.h"

namespace Glory
{
	AssetReferencePropertySerializer::AssetReferencePropertySerializer() : PropertySerializer(SerializedType::ST_Asset)
	{
	}

	AssetReferencePropertySerializer::~AssetReferencePropertySerializer()
	{
	}

	void AssetReferencePropertySerializer::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
		UUID uuid = *(UUID*)serializedProperty->MemberPointer();
		out << YAML::Key << serializedProperty->Name();
		out << YAML::Value << (uint64_t)uuid;
	}

	void AssetReferencePropertySerializer::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		UUID* pUUIDMember = (UUID*)serializedProperty->MemberPointer();
		UUID uuid = object.as<uint64_t>();
		*pUUIDMember = uuid;
		AssetManager::GetAsset(uuid, [](Resource*) {});
	}

	void AssetReferencePropertySerializer::Deserialize(std::any& out, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		UUID uuid = object.as<uint64_t>();
		out = uuid;
	}
}
