#include "AssetDatabase.h"

namespace Glory
{
	std::unordered_map<UUID, AssetLocation> AssetDatabase::m_AssetLocations;

	const AssetLocation* AssetDatabase::GetAssetLocation(UUID uuid)
	{
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
		{
			//throw new std::exception("Asset not found!");
			return nullptr;
		}
		return &m_AssetLocations[uuid];
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
