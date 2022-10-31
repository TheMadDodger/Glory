#include "AssetReferencePropertySerializer.h"
#include "AssetManager.h"
#include "AssetReference.h"

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

	void AssetReferencePropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		void* pAssetRefAddress = pFieldData->GetAddress(data);

		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)pAssetRefAddress;
		UUID uuid = pReferenceMember->AssetUUID();

		const std::string& name = pFieldData->Name();

		if (name == "")
		{
			out << (uint64_t)uuid;
			return;
		}

		out << YAML::Key << name;
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

	void AssetReferencePropertySerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		void* pAssetRefAddress = pFieldData->GetAddress(data);

		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)pAssetRefAddress;
		UUID uuid = object.as<uint64_t>();

		pReferenceMember->SetUUID(uuid);
	}
}
