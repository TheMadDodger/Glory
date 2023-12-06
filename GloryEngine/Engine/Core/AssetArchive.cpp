#include "AssetArchive.h"
#include "BinaryStream.h"
#include "Resource.h"
#include "ResourceType.h"
#include "Debug.h"

#include <sstream>

namespace Glory
{
	AssetArchive::AssetArchive(): m_pStream(nullptr), m_Version(), m_Owned()
	{
	}

	AssetArchive::AssetArchive(BinaryStream* pStream, bool isNew) : m_pStream(pStream), m_Version(), m_Owned()
	{
		if (isNew)
		{
			m_Version = Version::Parse(GloryCoreVersion);
			WriteVersion();
		}
		else
			ReadVersion();
	}

	AssetArchive::~AssetArchive()
	{
		if (!m_pStream) return;
		m_pStream->Close();
		m_pStream = nullptr;

		for (size_t i = 0; i < m_pResources.size(); ++i)
		{
			if (!m_Owned.IsSet(i)) continue;
			delete m_pResources[i];
		}
		m_pResources.clear();
	}

	bool AssetArchive::VerifyVersion()
	{
		const Version currentVersion = Version::Parse(GloryCoreVersion);
		return Version::Compare(m_Version, currentVersion) == 0;
	}

	void AssetArchive::Serialize(Resource* pResource)
	{
		const size_t subResourcesCount = pResource->SubResourceCount();

		std::type_index type = typeid(Resource);
		if (!pResource->GetType(0, type)) return;
		const uint32_t typeHash = ResourceTypes::GetHash(type);

		/* Write name, type and sub resource count */
		m_pStream->Write(pResource->GetUUID()).Write(pResource->Name()).Write(typeHash).Write(subResourcesCount);

		/* Write the resource */
		pResource->Serialize(*m_pStream);

		/* Write the sub resources */
		for (size_t i = 0; i < subResourcesCount; ++i)
		{
			Resource* pSubResource = pResource->Subresource(i);
			Serialize(pSubResource);
		}
	}

	void AssetArchive::Deserialize()
	{
		if (!VerifyVersion())
		{
			std::string versionStr;
			m_Version.GetVersionString(versionStr);
			std::stringstream str;
			str << "Compiled asset archive was built with a different core/runtime version (" << versionStr << ") than the current version " << GloryCoreVersion;
			Debug::LogFatalError(str.str());
			return;
		}

		while (!m_pStream->Eof())
		{
			ReadResource();
		}

		m_Owned.Reserve(m_pResources.size());
		m_Owned.SetAll();
	}

	size_t AssetArchive::Size() const
	{
		return m_pResources.size();
	}

	Resource* AssetArchive::Get(size_t index) const
	{
		if (!m_Owned.IsSet(index))
		{
			Debug::LogError("Resource already claimed!");
			return nullptr;
		}
		m_Owned.UnSet(index);
		return m_pResources[index];
	}

	AssetArchive::operator bool() const
	{
		return !!m_pStream;
	}

	void AssetArchive::WriteVersion()
	{
		m_pStream->Write(m_Version);
	}

	void AssetArchive::ReadVersion()
	{
		m_pStream->Read(m_Version);
	}

	Resource* AssetArchive::ReadResource()
	{
		std::string name;
		uint32_t typeHash = 0;
		size_t subResourcesCount = 0;
		UUID uuid = 0;
		m_pStream->Read(uuid).Read(name).Read(typeHash).Read(subResourcesCount);

		const ResourceType* pType = ResourceType::GetResourceType(typeHash);
		Resource* pResource = pType->Create(uuid, name);

		pResource->Deserialize(*m_pStream);
		m_pResources.push_back(pResource);

		for (size_t i = 0; i < subResourcesCount; ++i)
		{
			Resource* pSubResource = ReadResource();
			/* This should be UUID and not resource directly! */
			pResource->AddSubresource(pSubResource, pSubResource->Name());
		}

		return pResource;
	}
}