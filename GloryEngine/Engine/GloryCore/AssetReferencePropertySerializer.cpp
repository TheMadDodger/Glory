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

	void AssetReferencePropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		UUID uuid = object.as<uint64_t>();
		pReferenceMember->SetUUID(uuid);
	}
}
