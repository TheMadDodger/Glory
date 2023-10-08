#include "AssetCompiler.h"
#include "EditorAssetDatabase.h"

#include <AssetDatabase.h>
#include <JobManager.h>
#include <AssetManager.h>
#include <Importer.h>
#include <BinaryStream.h>
#include <AssetArchive.h>
#include <ProjectSpace.h>

namespace Glory::Editor
{
	std::map<UUID, AssetCompiler::AssetData> AssetCompiler::m_AssetDatas;
	ThreadedVector<UUID> AssetCompiler::m_CompilingAssets;

	Jobs::JobPool<bool, const AssetCompiler::AssetData>* CompilationJobPool = nullptr;

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

	void AssetCompiler::CompileAssets()
	{
		CompileAssets(EditorAssetDatabase::UUIDs());
	}

	void AssetCompiler::CompileAssets(const std::vector<UUID>& ids)
	{
		if (!CompilationJobPool)
			CompilationJobPool = Jobs::JobManager::Run<bool, const AssetData>();

		CompilationJobPool->StartQueue();
		for (UUID id : ids)
		{
			/* Get the root asset */
			while (!m_AssetDatas.at(id).Location.SubresourcePath.empty())
			{
				const UUID parentID = EditorAssetDatabase::FindAssetUUID(m_AssetDatas.at(id).Location.Path);
				if (!parentID)
				{
					std::stringstream str;
					str << "AssetCoompiler: Failed to get parent of " << id;
					Debug::LogWarning(str.str());
					break;
				}
				id = parentID;
			}

			const AssetData& data = m_AssetDatas.at(id);
			DispatchCompilationJob(data);
		}
		CompilationJobPool->EndQueue();
	}

	void AssetCompiler::DispatchCompilationJob(const AssetData& asset)
	{
		if (m_CompilingAssets.Contains(asset.Meta.ID())) return;
		m_CompilingAssets.push_back(asset.Meta.ID());
		CompilationJobPool->QueueJob(CompileJob, asset);
	}

	bool AssetCompiler::CompileJob(const AssetData asset)
	{
		std::filesystem::path path = Game::GetAssetPath();
		path.append(asset.Location.Path);

		if (!std::filesystem::exists(path))
			path = asset.Location.Path;

		/* Try get the asset if its already loaded */
		Resource* pResource = AssetManager::FindResource(asset.Meta.ID());
		if (!pResource)
		{
			/* Import the resource */
			pResource = Importer::Import(path, nullptr);
			if (!pResource)
			{
				/* TODO: Log this on main thread */
				//std::stringstream str;
				//str << "AssetCompiler: Failed to compile asset " << asset.Meta.ID() << " there was an error when importing the asset.";
				//Debug::LogError(str.str());
				m_CompilingAssets.Erase(asset.Meta.ID());
				return false;
			}

			/* Insert the loaded asset into the manager */
			AssetManager::AddLoadedResource(pResource);
		}

		/* Serialize the resource into a binary file */
		std::filesystem::path compiledPath = ProjectSpace::GetOpenProject()->CachePath();
		compiledPath.append("CompiledAssets").append(std::to_string(asset.Meta.ID())).replace_extension(".gag");
		BinaryFileStream stream{ compiledPath };
		{
			AssetArchive archive{ &stream };
			archive.Serialize(pResource);
		}

		/* We're done here */
		m_CompilingAssets.Erase(asset.Meta.ID());
		return true;
	}
}
