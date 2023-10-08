#pragma once
#include "GloryEditor.h"

#include <vector>
#include <map>
#include <UUID.h>
#include <AssetLocation.h>
#include <ThreadedVar.h>

namespace Glory::Editor
{
	class AssetCompiler
	{
	public:
		GLORY_EDITOR_API static void CompileAssetDatabase();
		GLORY_EDITOR_API static void CompileAssetDatabase(UUID id);
		GLORY_EDITOR_API static void CompileAssetDatabase(const std::vector<UUID>& ids);
		GLORY_EDITOR_API static void CompileNewAssets();
		GLORY_EDITOR_API static void CompileAssets();
		GLORY_EDITOR_API static void CompileAssets(const std::vector<UUID>& ids);

		struct AssetData
		{
			AssetLocation Location;
			ResourceMeta Meta;
		};

	private:
		static std::map<UUID, AssetData> m_AssetDatas;
		static ThreadedVector<UUID> m_CompilingAssets;

	private:
		static void DispatchCompilationJob(const AssetData& asset);
		static bool CompileJob(const AssetData asset);
		static std::filesystem::path GenerateCompiledAssetPath(const UUID uuid);
	};
}
