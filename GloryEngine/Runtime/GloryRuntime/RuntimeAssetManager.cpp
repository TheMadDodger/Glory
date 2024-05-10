#include "RuntimeAssetManager.h"

#include <Engine.h>
#include <Debug.h>
#include <AssetDatabase.h>
#include <BinaryStream.h>
#include <AssetArchive.h>
#include <Hash.h>

namespace Glory
{
	RuntimeAssetManager::RuntimeAssetManager(Engine* pEngine) : AssetManager(pEngine)
	{
	}

	RuntimeAssetManager::~RuntimeAssetManager()
	{
		m_pLoadedAssets.ForEach([](const UUID& key, Resource* value)
		{
			delete value;
		});

		m_pLoadedAssets.Clear();
		m_LoadedArchives.Clear();
	}

	void RuntimeAssetManager::GetAsset(UUID uuid, std::function<void(Resource*)> callback)
	{
		if (uuid == 0) return;
		Resource* pResource = FindResource(uuid);
		if (pResource)
		{
			callback(pResource);
			return;
		}

		m_pEngine->GetDebug().LogError("Loading assets at runtime is currently not supported, have a look at additive scene loading as a workaround.");
		return;
	}

	Resource* RuntimeAssetManager::GetOrLoadAsset(UUID uuid)
	{
		if (uuid == 0) return nullptr;
		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		m_pEngine->GetDebug().LogError("Loading assets at runtime is currently not supported, have a look at additive scene loading as a workaround.");
		return nullptr;
	}

	Resource* RuntimeAssetManager::GetAssetImmediate(UUID uuid)
	{
		if (uuid == 0) return nullptr;
		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		m_pEngine->GetDebug().LogError("Loading assets at runtime is currently not supported, have a look at additive scene loading as a workaround.");
		return nullptr;
	}

	void RuntimeAssetManager::UnloadAsset(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return;
		m_pLoadedAssets.DoErase(uuid, [](Resource** pResource) { if (*pResource) delete* pResource; });
	}

	Resource* RuntimeAssetManager::FindResource(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return nullptr;
		return m_pLoadedAssets[uuid];
	}

	void RuntimeAssetManager::AddLoadedResource(Resource* pResource, UUID uuid)
	{
		pResource->SetResourceUUID(uuid);
		AddLoadedResource(pResource);
	}

	void RuntimeAssetManager::AddLoadedResource(Resource* pResource)
	{
		UnloadAsset(pResource->GetUUID());
		m_pLoadedAssets.Set(pResource->GetUUID(), pResource);
	}

	const AssetArchive* RuntimeAssetManager::GetOrLoadArchive(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::stringstream str;
			str << "Failed to load asset archive at path " << path << " file not found!";
			m_pEngine->GetDebug().LogError(str.str());
			return nullptr;
		}

		/* Load as archive */
		BinaryFileStream stream{ path };
		AssetArchive archive{ &stream };
		archive.Deserialize(m_pEngine);
		const std::string& str = path.string();
		const uint32_t hash = Hashing::Hash(str.data());
		m_LoadedArchives.Emplace(hash, std::move(archive));
		stream.Close();
		return &m_LoadedArchives.at(hash);
	}

	void RuntimeAssetManager::AddAssetArchive(uint32_t hash, AssetArchive&& archive)
	{
		m_LoadedArchives.Emplace(hash, std::move(archive));
	}

	void RuntimeAssetManager::Initialize()
	{
	}
}
