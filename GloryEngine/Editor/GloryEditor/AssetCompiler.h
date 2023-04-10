#pragma once
#include "GloryEditor.h"

#include <vector>
#include <map>
#include <UUID.h>
#include <AssetLocation.h>

namespace Glory::Editor
{
	class AssetCompiler
	{
	public:
		GLORY_EDITOR_API static void CompileAssetDatabase();
		GLORY_EDITOR_API static void CompileAssetDatabase(UUID id);
		GLORY_EDITOR_API static void CompileAssetDatabase(const std::vector<UUID>& ids);

	private:
		struct AssetData
		{
			AssetLocation Location;
			ResourceMeta Meta;
		};

		static std::map<UUID, AssetData> m_AssetDatas;
	};
}
