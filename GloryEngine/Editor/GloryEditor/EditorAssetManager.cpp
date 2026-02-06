#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "EditorAssetCallbacks.h"
#include "AssetCompiler.h"
#include "EditorAssetDatabase.h"
#include "TextureImporter.h"
#include "Tumbnail.h"

#include <Engine.h>
#include <AssetDatabase.h>
#include <Undo.h>

namespace Glory::Editor
{
	EditorAssetManager::EditorAssetManager(EditorApplication* pApplication):
		AssetManager(pApplication->GetEngine()), m_pResourceLoadingPool(nullptr)
	{
	}

	EditorAssetManager::~EditorAssetManager()
	{
		m_pLoadedAssets.ForEach([](const UUID& key, Resource* value)
		{
			delete value;
		});

		m_pLoadedAssets.Clear();

		m_pResourceLoadingPool = nullptr;
	}

	void EditorAssetManager::GetAsset(UUID uuid, std::function<void(Resource*)> callback)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource)
		{
			callback(pResource);
			return;
		}

		if (m_AssetLoadedCallbacks.Contains(uuid))
		{
			if (callback != NULL)
			{
				m_AssetLoadedCallbacks.Do(uuid, [&](std::vector<std::function<void(Resource*)>>* callbacks)
				{
					callbacks->push_back(callback);
				});
			}
			return;
		}
		else if (callback != NULL) m_AssetLoadedCallbacks.Set(uuid, { callback });

		m_pResourceLoadingPool->QueueSingleJob([this](UUID uuid) { return LoadResourceJob(uuid); }, uuid);
	}

	Resource* EditorAssetManager::GetOrLoadAsset(UUID uuid)
	{
		if (m_pLoadingAssets.Contains(uuid)) return nullptr;

		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		if (m_AssetLoadedCallbacks.Contains(uuid)) return nullptr;
		m_AssetLoadedCallbacks.Set(uuid, { [](Resource*) {} });
		m_pResourceLoadingPool->QueueSingleJob([this](UUID uuid) { return LoadResourceJob(uuid); }, uuid);
		return nullptr;
	}

	Resource* EditorAssetManager::GetAssetImmediate(UUID uuid)
	{
		while (true)
		{
			LoadingLock lock{ this, uuid };
			if (lock.IsValid) break;
		}

		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		return LoadAsset(uuid);
	}

	void EditorAssetManager::ReloadAsset(UUID uuid)
	{
		UnloadAsset(uuid);
		GetAsset(uuid, [&](Resource* pResource)
		{
			EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetReloaded, uuid, pResource);
		});
	}

	void EditorAssetManager::UnloadAsset(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return;
		m_pLoadedAssets.DoErase(uuid, [](Resource** pResource) { if (*pResource) delete* pResource; });
	}

	Resource* EditorAssetManager::FindResource(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return nullptr;
		return m_pLoadedAssets[uuid];
	}

	void EditorAssetManager::AddLoadedResource(Resource* pResource, UUID uuid)
	{
		pResource->SetResourceUUID(uuid);
		AddLoadedResource(pResource);
	}

	void EditorAssetManager::AddLoadedResource(Resource* pResource)
	{
		const bool replaced = m_pLoadedAssets.Do(pResource->GetUUID(), [pResource](Resource** pOther) {
			if (*pOther == pResource) return;
			delete *pOther;
			*pOther = pResource;
		});

		if (replaced) return;
		m_pLoadedAssets.Set(pResource->GetUUID(), pResource);
	}

	bool EditorAssetManager::IsLoading(UUID uuid)
	{
		return m_pLoadingAssets.Contains(uuid);
	}

	void EditorAssetManager::GetAllLoading(std::vector<UUID>& out)
	{
		m_pLoadingAssets.ForEach([&out](const UUID& uuid) {
			out.push_back(uuid);
		});
	}

	bool EditorAssetManager::LoadResourceJob(UUID uuid)
	{
		Resource* pResource = LoadAsset(uuid);
		m_ResourceLoadedCallbacks.push(CallbackData(uuid, pResource));
		return pResource;
	}

	Resource* EditorAssetManager::LoadAsset(UUID uuid)
	{
		if (!AssetCompiler::IsCompilingAsset(uuid))
		{
			return FindResource(uuid);
		}

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (AssetCompiler::IsCompilingAsset(uuid)) continue;
			return FindResource(uuid);
		}

		return nullptr;
	}

	void EditorAssetManager::Initialize()
	{
		m_pResourceLoadingPool = m_pEngine->Jobs().Run<bool, UUID>();

		Undo::RegisterChangeHandler(".gtex", "", [this](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			const UUID uuid = EditorAssetDatabase::FindAssetUUID(file.Path().string());
			if (uuid == 0) return;
			Resource* pResource = FindResource(uuid);
			if (!pResource) return;
			TextureData* pTexture = static_cast<TextureData*>(pResource);
			TextureImporter::LoadIntoTexture(file, pTexture);
			Tumbnail::SetDirty(uuid);
		});
	}

	void EditorAssetManager::RunCallbacks()
	{
		CallbackData callbackData;
		while (m_ResourceLoadedCallbacks.Pop(callbackData))
		{
			m_AssetLoadedCallbacks.Do(callbackData.m_UUID, [&](const std::vector<std::function<void(Resource*)>>& callbacks)
			{
				for (size_t i = 0; i < callbacks.size(); i++)
				{
					callbacks[i](callbackData.m_pResource);
				}
			});
			m_AssetLoadedCallbacks.Erase(callbackData.m_UUID);
		}
	}

	CallbackData::CallbackData()
		: m_UUID(0), m_pResource(nullptr)
	{
	}

	CallbackData::CallbackData(UUID uuid, Resource* pResource)
		: m_UUID(uuid), m_pResource(pResource)
	{
	}

	EditorAssetManager::LoadingLock::LoadingLock(EditorAssetManager* pManager, UUID uuid) :
		m_pManager(pManager), m_UUID(uuid), IsValid(false)
	{
		if (pManager->m_pLoadingAssets.Contains(uuid)) return;
		pManager->m_pLoadingAssets.push_back(uuid);
		IsValid = true;
	}

	EditorAssetManager::LoadingLock::~LoadingLock()
	{
		if (!IsValid) return;
		m_pManager->m_pLoadingAssets.Erase(m_UUID);
		IsValid = false;
	}
}
