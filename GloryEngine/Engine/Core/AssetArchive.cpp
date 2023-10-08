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
		const size_t subResourcesCount = pResource->SubResourceCount();

		std::type_index type = typeid(Resource);
		if (!pResource->GetType(0, type)) return;
		const uint32_t typeHash = ResourceType::GetHash(type);

		/* Write name, type and sub resource count */
		m_pStream->Write(pResource->Name()).Write(typeHash).Write(subResourcesCount);

		/* Write the resource */
		pResource->Serialize(*m_pStream);

		/* Write the sub resources */
		for (size_t i = 0; i < subResourcesCount; ++i)
		{
			Resource* pSubResource = pResource->Subresource(i);
			Serialize(pSubResource);
		}
	}

	void AssetArchive::Deserialize(Resource* pResource)
	{

	}
}