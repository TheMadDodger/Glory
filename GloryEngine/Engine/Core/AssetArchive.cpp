#include "AssetArchive.h"
#include "BinaryStream.h"
#include "Resource.h"
#include "ResourceType.h"
#include "Debug.h"
#include "Engine.h"

#include <sstream>

namespace Glory
{
	AssetArchive::AssetArchive(BinaryStream* pStream, bool isNew): m_pStream(pStream), m_Version(), m_Owned()
	{
		if (isNew)
		{
			m_Version = Version::Parse(GloryCoreVersion);
			WriteVersion();
		}
		else
			ReadVersion();
	}

	AssetArchive::AssetArchive(AssetArchive&& other) noexcept:
		m_pStream(other.m_pStream), m_Version(other.m_Version), m_Owned(std::move(other.m_Owned))
	{
		other.m_pStream = nullptr;
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
		std::type_index type = typeid(Resource);
		if (!pResource->GetType(0, type)) return;
		const uint32_t typeHash = ResourceTypes::GetHash(type);

		/* Write ID, name and type */
		m_pStream->Write(pResource->GetUUID()).Write(pResource->Name()).Write(typeHash);

		/* Write the resource */
		pResource->Serialize(*m_pStream);
	}

	void AssetArchive::Deserialize(Engine* pEngine)
	{
		if (!VerifyVersion())
		{
			std::string versionStr;
			m_Version.GetVersionString(versionStr);
			std::stringstream str;
			str << "Compiled asset archive was built with a different core/runtime version (" << versionStr << ") than the current version " << GloryCoreVersion;
			pEngine->GetDebug().LogFatalError(str.str());
			return;
		}

		while (!m_pStream->Eof())
		{
			ReadResource(pEngine);
		}

		m_Owned.Reserve(m_pResources.size());
		m_Owned.SetAll();
	}

	size_t AssetArchive::Size() const
	{
		return m_pResources.size();
	}

	Resource* AssetArchive::Get(Engine* pEngine, size_t index) const
	{
		if (!m_Owned.IsSet(index))
		{
			pEngine->GetDebug().LogError("Resource already claimed!");
			return nullptr;
		}
		m_Owned.UnSet(index);
		return m_pResources[index];
	}

	void AssetArchive::WriteVersion()
	{
		m_pStream->Write(m_Version);
	}

	void AssetArchive::ReadVersion()
	{
		m_pStream->Read(m_Version);
	}

	Resource* AssetArchive::ReadResource(Engine* pEngine)
	{
		std::string name;
		uint32_t typeHash = 0;
		UUID uuid = 0;
		m_pStream->Read(uuid).Read(name).Read(typeHash);

		const ResourceType* pType = pEngine->GetResourceTypes().GetResourceType(typeHash);
		if (!pType)
		{
			throw new std::exception("Non existing resource type");
		}

		Resource* pResource = pType->Create();

		pResource->SetName(name);
		pResource->SetResourceUUID(uuid);

		pResource->Deserialize(*m_pStream);
		m_pResources.push_back(pResource);

		return pResource;
	}
}