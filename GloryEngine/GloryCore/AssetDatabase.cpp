#include "AssetDatabase.h"

namespace Glory
{
	std::unordered_map<UUID, AssetLocation> AssetDatabase::m_AssetLocations;
	std::vector<ResourceMeta> AssetDatabase::m_Metas;


	const AssetLocation* AssetDatabase::GetAssetLocation(UUID uuid)
	{
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
		{
			//throw new std::exception("Asset not found!");
			return nullptr;
		}

		const AssetLocation* pLocation = &m_AssetLocations[uuid];
		return pLocation;
	}

	bool AssetDatabase::AssetExists(UUID uuid)
	{
		return m_AssetLocations.find(uuid) != m_AssetLocations.end();
	}

	void AssetDatabase::InsertAsset(const std::string& path, const ResourceMeta& meta)
	{
		m_Metas.push_back(meta);
		uint64_t uuid = meta.ReadUUID();
		m_AssetLocations[uuid] = AssetLocation(path, &m_Metas[m_Metas.size() - 1]);
	}

	void AssetDatabase::Initialize()
	{
	}

	void AssetDatabase::Destroy()
	{
		m_AssetLocations.clear();
	}

	void AssetDatabase::ExportEditor(std::basic_ostream<char, std::char_traits<char>>& ostream)
	{
	}

	void AssetDatabase::ExportBuild(std::basic_ostream<char, std::char_traits<char>>& ostream)
	{
		
	}

	AssetDatabase::AssetDatabase() {}

	AssetDatabase::~AssetDatabase() {}
}
