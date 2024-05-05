#include "EditorResourceManager.h"
#include "EditorAssetDatabase.h"
#include "AssetCompiler.h"
#include "EditableResource.h"
#include "ProjectSpace.h"

#include <Engine.h>
#include <AssetManager.h>
#include <Importer.h>
//#include <AssetLoader.h>

namespace Glory::Editor
{
	EditorResourceManager::EditorResourceManager(Engine* pEngine):
		m_pEngine(pEngine) {}

	EditorResourceManager::~EditorResourceManager()
	{
		for (auto itor = m_pEditableResources.begin(); itor != m_pEditableResources.end(); ++itor)
		{
			delete itor->second;
		}
		m_pEditableResources.clear();
	}

	//void EditorResourceManager::GetAsset(const UUID uuid, std::function<void(Resource*)> callback)
	//{
	//	if (m_Callbacks.find(uuid) != m_Callbacks.end())
	//		return;

	//	uint32_t& hash = m_Hashes[uuid];
	//	auto hashItor = m_Hashes.find(uuid);
	//	if (!hash)
	//	{
	//		ResourceMeta meta;
	//		if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta))
	//			return callback(nullptr);
	//		hash = meta.Hash();
	//	}

	//	Resource* pResource = m_pEngine->GetAssetManager().GetOrLoadAsset(hash);
	//	if (pResource)
	//	{
	//		callback(pResource);
	//		return;
	//	}

	//	/*if (m_pEngine->GetResources().Manager(hash)->IsLoaded(uuid))
	//	{
	//		Resource* pResource = m_pEngine->GetResources().Manager(hash)->GetBase(uuid);
	//		callback(pResource);
	//		return;
	//	}*/

	//	if (!AssetCompiler::IsCompiled(uuid))
	//	{
	//		AssetCompiler::CompileAssets({ uuid });
	//		m_Callbacks.emplace(uuid, callback);
	//		return;
	//	}

	//	if (!m_pEngine->GetAssetLoader().RequestLoad(uuid))
	//		return callback(nullptr);

	//	m_Callbacks.emplace(uuid, callback);
	//}

	/*void EditorResourceManager::RunCallbacks()
	{
		std::vector<UUID> finishedCallbacks;
		for (auto itor = m_Callbacks.begin(); itor != m_Callbacks.end(); ++itor)
		{
			const UUID uuid = itor->first;
			const uint32_t hash = m_Hashes.at(uuid);
			if (!m_pEngine->GetResources().Manager(hash)->IsLoaded(uuid))
				continue;
			Resource* pResource = m_pEngine->GetResources().Manager(hash)->GetBase(uuid);
			itor->second(pResource);
			finishedCallbacks.push_back(uuid);
		}

		for (size_t i = 0; i < finishedCallbacks.size(); ++i)
		{
			m_Callbacks.erase(finishedCallbacks[i]);
		}
	}*/

	EditableResource* EditorResourceManager::GetEditableResource(const UUID uuid)
	{
		auto itor = m_pEditableResources.find(uuid);
		if (itor != m_pEditableResources.end())
			return itor->second;

		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location))
			return nullptr;

		std::filesystem::path path = location.Path;
		if (!std::filesystem::exists(path))
		{
			path = ProjectSpace::GetOpenProject()->RootPath();
			path.append("Assets").append(location.Path);
		}

		EditableResource* pResource = Importer::CreateEditableResource(path);
		if (!pResource) return nullptr;
		pResource->SetResourceUUID(uuid);
		m_pEditableResources.emplace(uuid, pResource);
		return pResource;
	}

	void EditorResourceManager::Save()
	{
		for (auto itor = m_pEditableResources.begin(); itor != m_pEditableResources.end(); ++itor)
		{
			if (itor->second->IsDirty())
				itor->second->Save();
		}
	}

	void EditorResourceManager::ReloadEditableAsset(const UUID uuid)
	{
		auto itor = m_pEditableResources.find(uuid);
		if (itor == m_pEditableResources.end())
			return;

		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location))
			return;

		std::filesystem::path path = ProjectSpace::GetOpenProject()->RootPath();
		path.append("Assets").append(location.Path);

		itor->second->Reload(path);
	}
}
