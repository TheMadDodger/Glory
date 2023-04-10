#include "AssetCompiler.h"
#include "EditorAssetDatabase.h"

#include <AssetDatabase.h>

namespace Glory::Editor
{
	std::map<UUID, AssetCompiler::AssetData> AssetCompiler::m_AssetDatas;

	void AssetCompiler::CompileAssetDatabase()
	{
		CompileAssetDatabase(EditorAssetDatabase::UUIDs());
	}

	void AssetCompiler::CompileAssetDatabase(UUID id)
	{
		CompileAssetDatabase(std::vector<UUID>{ id });
	}

	void AssetCompiler::CompileAssetDatabase(const std::vector<UUID>& ids)
	{
		AssetDatabase::WriteLock lock;

		for (UUID id : ids)
		{
			AssetData& data = m_AssetDatas[id];

			if (!EditorAssetDatabase::GetAssetLocation(id, data.Location))
			{
				AssetDatabase::Remove(id);
				m_AssetDatas.erase(id);
				continue;
			}

			if (!EditorAssetDatabase::GetAssetMetadata(id, data.Meta))
			{
				AssetDatabase::Remove(id);
				m_AssetDatas.erase(id);
				continue;
			}
			AssetDatabase::SetAsset(data.Location, data.Meta);
		}
	}
}
