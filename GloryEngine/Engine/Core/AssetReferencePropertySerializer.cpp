#include "AssetReferencePropertySerializer.h"
#include "AssetManager.h"
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

	void AssetReferencePropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		UUID uuid = pReferenceMember->AssetUUID();

		if (name.empty())
		{
			node.Set((uint64_t)uuid);
			return;
		}

		node[name].Set((uint64_t)uuid);
	}

	void AssetReferencePropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsScalar()) return;

		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		const UUID uuid = node.As<uint64_t>();
		pReferenceMember->SetUUID(uuid);
	}
}
