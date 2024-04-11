#include "AssetCompiler.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"

#include <Debug.h>
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
	ThreadedUMap<std::filesystem::path, ImportedResource> ImportedResources;

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
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		AssetDatabase& assetDatabase = pEngine->GetAssetDatabase();
		AssetManager& assetManager = pEngine->GetAssetManager();
		AssetDatabase::WriteLock lock{ &assetDatabase };

		for (UUID id : ids)
		{
			AssetData& data = m_AssetDatas[id];

			if (!EditorAssetDatabase::GetAssetLocation(id, data.Location))
			{
				assetDatabase.Remove(id);
				m_AssetDatas.erase(id);
				continue;
			}

			if (!EditorAssetDatabase::GetAssetMetadata(id, data.Meta))
			{
				assetDatabase.Remove(id);
				m_AssetDatas.erase(id);
				continue;
			}

			AssetLocation location = data.Location;
			location.Index = 0;
			location.Path = GenerateCompiledAssetPath(id).string();
			assetDatabase.SetAsset(location, data.Meta);
		}
	}

	void AssetCompiler::CompileNewAssets()
	{
		std::vector<UUID> ids = EditorAssetDatabase::UUIDs();
		std::vector<UUID> newIDs;

		for (UUID id : ids)
		{
			const std::filesystem::path path = GenerateCompiledAssetPath(id);
			const bool exists = std::filesystem::exists(path.string());
			if (exists) continue;
			newIDs.push_back(id);
		}

		CompileAssets(newIDs);
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
			const AssetData& data = m_AssetDatas.at(id);
			DispatchCompilationJob(data);
		}
		CompilationJobPool->EndQueue();
	}

	bool AssetCompiler::IsBusy()
	{
		CompilationJobPool->HasTasksInQueue();
		return m_CompilingAssets.Size();
	}

	void AssetCompiler::DispatchCompilationJob(const AssetData& asset)
	{
		if (m_CompilingAssets.Contains(asset.Meta.ID())) return;
		/* Don't compile scenes */
		if (asset.Meta.Extension() == ".gscene") return;

		m_CompilingAssets.push_back(asset.Meta.ID());
		CompilationJobPool->QueueJob(CompileJob, asset);
	}

	bool AssetCompiler::CompileJob(const AssetData asset)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		AssetDatabase& assetDatabase = pEngine->GetAssetDatabase();
		AssetManager& assetManager = pEngine->GetAssetManager();

		std::filesystem::path path = assetDatabase.GetAssetPath();
		path.append(asset.Location.Path);

		if (!std::filesystem::exists(path))
			path = asset.Location.Path;

		const UUID uuid = asset.Meta.ID();

		/* Try get the asset if its already loaded */
		Resource* pResource = assetManager.FindResource(uuid);
		if (!pResource)
		{
			ImportedResources.Do([&path, &uuid](std::unordered_map<std::filesystem::path, ImportedResource>& data) {
				auto itor = data.find(path);
				if (itor != data.end()) return;

				/* Import the resource */
				ImportedResource resource = Importer::Import(path);
				data.emplace(path, std::move(resource));
			});

			ImportedResources.Do(path, [asset, &pResource](ImportedResource& resource) {
				ImportedResource* pChild = resource.ChildFromPath(asset.Location.SubresourcePath);
				if (!pChild) return;
				pResource = **pChild;
			});

			if (!pResource)
			{
				std::stringstream str;
				str << "AssetCompiler: Failed to compile asset " << uuid << " there was an error when importing the asset.";
				pEngine->GetDebug().LogError(str.str());
				m_CompilingAssets.Erase(uuid);
				return false;
			}

			/* Insert the loaded asset into the manager */
			pResource->SetName(asset.Meta.Name());
			assetManager.AddLoadedResource(pResource, uuid);
		}

		/* Serialize the resource into a binary file */
		/* @todo: Disabled until asset loading works, so for now assets are recompiled every time */
		//const std::filesystem::path compiledPath = GenerateCompiledAssetPath(uuid);
		//{
		//	BinaryFileStream stream{ compiledPath };
		//	AssetArchive archive{ &stream };
		//	archive.Serialize(pResource);
		//}

		std::stringstream str;
		str << "AssetCompiler: Compiled asset " << uuid;
		pEngine->GetDebug().LogInfo(str.str());

		/* We're done here */
		m_CompilingAssets.Erase(uuid);
		return true;
	}

	std::filesystem::path AssetCompiler::GenerateCompiledAssetPath(const UUID uuid)
	{
		std::filesystem::path compiledPath = ProjectSpace::GetOpenProject()->CachePath();
		compiledPath.append("CompiledAssets").append(std::to_string(uuid)).replace_extension(".gcag");
		return compiledPath;
	}
}
