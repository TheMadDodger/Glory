#include "AssetReferencePropertySerializer.h"
#include "AssetReference.h"

namespace Glory
{
	AssetReferencePropertySerializer::AssetReferencePropertySerializer(Serializers* pSerializers):
		PropertySerializer(pSerializers, SerializedType::ST_Asset)
	{
	}

	AssetReferencePropertySerializer::~AssetReferencePropertySerializer()
	{
	}

	void AssetReferencePropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		UUID uuid = pReferenceMember->AssetUUID();

		if (name.empty())
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
