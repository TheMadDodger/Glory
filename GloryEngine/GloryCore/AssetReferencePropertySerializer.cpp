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
		Object* pObject = serializedProperty->ObjectReference();
		UUID uuid = pObject != nullptr ? pObject->GetUUID() : 0;
		out << YAML::Key << serializedProperty->Name();
		out << YAML::Value << (uint64_t)uuid;
	}

	void AssetReferencePropertySerializer::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		Object** pObjectMember = (Object**)serializedProperty->MemberPointer();
		UUID uuid = object.as<uint64_t>();
		Resource* pResource = AssetManager::GetAssetImmediate(uuid);
		*pObjectMember = pResource;
	}

	void AssetReferencePropertySerializer::Deserialize(std::any& out, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		UUID uuid = object.as<uint64_t>();
		out = AssetManager::GetAssetImmediate(uuid);
	}
}
