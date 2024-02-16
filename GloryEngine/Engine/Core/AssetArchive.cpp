#include "AssetArchive.h"
#include "BinaryStream.h"
#include "Resource.h"
#include "ResourceType.h"

namespace Glory
{
	AssetArchive::AssetArchive(BinaryStream* pStream) : m_pStream(pStream)
	{
	}

	AssetArchive::~AssetArchive()
	{
		if (!m_pStream) return;
		m_pStream->Close();
		m_pStream = nullptr;
	}

	void AssetArchive::Serialize(Resource* pResource)
	{
		std::type_index type = typeid(Resource);
		if (!pResource->GetType(0, type)) return;
		const uint32_t typeHash = ResourceTypes::GetHash(type);

		/* Write name, type and sub resource count */
		m_pStream->Write(pResource->Name()).Write(typeHash);

		/* Write the resource */
		pResource->Serialize(*m_pStream);
	}

	void AssetArchive::Deserialize(Resource* pResource)
	{

	}
}