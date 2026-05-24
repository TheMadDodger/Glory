#include "EditorResourceLoader.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "Undo.h"
#include "TextureImporter.h"
#include "EditorApplication.h"
#include "Thumbnails.h"
#include "Dispatcher.h"
#include "RemovedAssetsPopup.h"
#include "EditorSceneManager.h"
#include "EditorAssetCallbacks.h"

#include <Debug.h>
#include <JobManager.h>
#include <ThreadManager.h>
#include <JobWorkerPool.h>
#include <Resources.h>
#include <AssetDatabase.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <TextureData.h>
#include <PipelineData.h>
#include <MaterialData.h>
#include <PrefabData.h>

#include <GloryVersion.h>
#include <GloryAssert.h>

namespace Glory::Editor
{
	static Jobs::JobWorkerPool<bool, UUID, std::filesystem::path, Resource*>* ResourceLoaderJobPool = nullptr;

	CallbackData::CallbackData()
		: m_UUID(0), m_pResource(nullptr)
	{
	}

	CallbackData::CallbackData(UUID uuid, Resource* pResource)
		: m_UUID(uuid), m_pResource(pResource)
	{
	}

	EditorResourceLoader::EditorResourceLoader(EditorApplication* pApplication, Jobs::JobManager* pJobManager, Debug* pDebug):
		m_pApplication(pApplication), m_pJobManager(pJobManager),
		m_pDebug(pDebug), m_LoadedResources(nullptr), m_CachedResources(0ull)
	{
		ResourceLoaderJobPool = m_pJobManager->Run<bool, UUID, std::filesystem::path, Resource*>();
	}

	EditorResourceLoader::~EditorResourceLoader()
	{
		ResourceLoaderJobPool = nullptr;
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetUpdated, m_AssetUpdatedCallback);
	}

	void EditorResourceLoader::CheckResourceCache()
	{
		/* Ensures we release the resources once we are done with them instead of keeping them in memory */
		m_BuildingResourceCache = true;

		/* Block reference counting until we are done */
		m_pResources->SetAllowReferenceCounting(false);

		/* Go over all resources and check if they have a valid cache, if not it must be created now */
		EditorAssetDatabase::ForEachResource([this](const UUID id) {
			AssetLocation location;
			ResourceMeta meta;
			if (!EditorAssetDatabase::GetAssetLocation(id, location) || !EditorAssetDatabase::GetAssetMetadata(id, meta))
			{
				/* Corrupt entry in project file */
				m_ToRemoveAssets.push_back(id);
				return;
			}

			/* Skip resources that can't be cached */
			if (m_NonCachableResourceTypes.contains(meta.Hash())) return;

			std::filesystem::path cachePath;
			std::filesystem::path assetPath;
			if (ResourceHasValidCache(id, cachePath, assetPath) && CheckCacheVersion(cachePath)) return;

			m_ToCheckRemovedResources[assetPath].insert(id);

			if (m_AlreadyCompilingPaths.contains(assetPath)) return;
			m_AlreadyCompilingPaths.emplace(assetPath, 0);
			m_CompilingAssets.insert(id);

			/* Cache outdated or missing, we need to generate it to check for removed resources */
			ResourceLoaderJobPool->QueueSingleJob([this](UUID, std::filesystem::path assetPath, Resource*)
				{ return CompileJob(assetPath); }, id, assetPath, nullptr);
		});

		/* Keep updating until compilation is done */
		while (IsBusy())
			Update();

		m_BuildingResourceCache = false;
		m_pResources->SetAllowReferenceCounting(true);
	}

	void EditorResourceLoader::Initialize()
	{
		Undo::RegisterChangeHandler(".gtex", "", [this](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			const UUID uuid = EditorAssetDatabase::FindAssetUUID(file.Path().string());
			if (uuid == 0) return;
			Resource* pResource = m_pResources->GetResource(uuid);
			if (!pResource) return;
			TextureData* pTexture = static_cast<TextureData*>(pResource);
			TextureImporter::LoadIntoTexture(file, pTexture);
			EditorAssetDatabase::SetAssetDirty(pTexture);
		});

		AddTypeToLoadImmediately<PipelineData>();
		AddTypeToLoadImmediately<MaterialData>();
		AddTypeToLoadImmediately<TextureData>();
		AddTypeToLoadImmediately<PrefabData>();

		SetResourceNonCachable<PipelineData>();
		SetResourceNonCachable<GScene>();

		m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetDirty,
			[this](const AssetCallbackData& callback) {
				const std::filesystem::path cachePath = GenerateCompiledResourcePath(callback.m_UUID);
				if (std::filesystem::exists(cachePath))
					std::filesystem::remove(cachePath);
			});
	}

	bool EditorResourceLoader::IsBusy() const
	{
		return IsCompilingAssets() || IsCachingAssets() || IsLoadingResources();
	}

	bool EditorResourceLoader::IsCompilingAssets() const
	{
		return !m_CompilingAssets.empty();
	}

	bool EditorResourceLoader::IsCachingAssets() const
	{
		return !m_CachingItems.empty();
	}

	bool EditorResourceLoader::IsLoadingResources() const
	{
		return !m_LoadingResources.empty();
	}

	void EditorResourceLoader::CompilePipelines()
	{
		std::vector<UUID> ids;
		EditorAssetDatabase::GetAllAssetsOfType(ResourceTypes::GetHash<PipelineData>(), ids);

		/* Add a reference for each pipeline, keep the reference because we want them to be loaded at all times */
		for (auto& id : ids)
			m_pResources->AddReference(id);

		/* Trigger compilation job queueing */
		Update();

		/* Wait for compilation to become idle */
		while (IsBusy())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			Update();
		}
	}

	void EditorResourceLoader::CompileAssetDatabase()
	{
		CompileAssetDatabase(EditorAssetDatabase::UUIDs());
	}

	void EditorResourceLoader::CompileAssetDatabase(UUID id)
	{
		CompileAssetDatabase(std::vector<UUID>{ id });
	}

	void EditorResourceLoader::CompileAssetDatabase(const std::vector<UUID>& ids)
	{
		IEngine* pEngine = EditorApplication::GetInstance()->GetEngine();
		AssetDatabase& assetDatabase = pEngine->GetAssetDatabase();
		Resources& resources = pEngine->GetResources();
		AssetDatabase::WriteLock lock{ &assetDatabase };

		for (UUID id : ids)
		{
			AssetLocation location;
			ResourceMeta meta;

			if (!EditorAssetDatabase::GetAssetLocation(id, location))
			{
				assetDatabase.Remove(id);
				continue;
			}

			if (!EditorAssetDatabase::GetAssetMetadata(id, meta))
			{
				assetDatabase.Remove(id);
				continue;
			}

			location.Index = 0;
			location.Path = GenerateCompiledResourcePath(id).string();
			assetDatabase.SetAsset(location, meta);
		}
	}

	bool EditorResourceLoader::CompileSceneSettings(UUID uuid)
	{
		EditorSceneManager& sceneManager = m_pApplication->GetSceneManager();
		GScene* pScene = sceneManager.GetOpenScene(uuid);
		if (!pScene) return false;
		auto sceneFile = sceneManager.GetSceneFile(uuid);
		return CompileSceneSettings(pScene, **sceneFile);
	}

	bool EditorResourceLoader::CompileSceneSettings(GScene* pScene, Utils::NodeValueRef root)
	{
		if (!pScene) return false;

		SceneSettings& sceneSettings = pScene->Settings();
		auto settings = root["Settings"];
		if (!settings.Exists() || !settings.IsMap())
			return false;

		auto rendering = settings["Rendering"];
		if (!rendering.Exists() || !rendering.IsMap())
			return false;

		auto lighting = rendering["Lighting"];
		if (!lighting.Exists() || !lighting.IsMap())
			lighting.SetMap();

		auto skybox = lighting["Skybox"];
		sceneSettings.m_LightingSettings.m_Skybox = skybox.As<uint64_t>(0);
		auto irradiance = lighting["Irradiance"];
		sceneSettings.m_LightingSettings.m_IrradianceMap = irradiance.As<uint64_t>(0);

		auto ssao = rendering["SSAO"];
		if (!ssao.Exists() || !ssao.IsMap())
			ssao.SetMap();

		auto enable = ssao["Enable"];
		auto sampleRadius = ssao["SampleRadius"];
		auto sampleBias = ssao["SampleBias"];
		auto kernelSize = ssao["KernelSize"];
		auto blurType = ssao["BlurType"];
		auto blurSize = ssao["BlurSize"];
		auto separation = ssao["Separation"];
		auto binsSize = ssao["BinsSize"];
		auto magnitude = ssao["Magnitude"];
		auto contrast = ssao["Contrast"];

		sceneSettings.m_SSAOSettings.m_Enabled = enable.As<bool>(DefaultSSAO.m_Enabled);
		sceneSettings.m_SSAOSettings.m_SampleRadius = sampleRadius.As<float>(DefaultSSAO.m_SampleRadius);
		sceneSettings.m_SSAOSettings.m_SampleBias = sampleBias.As<float>(DefaultSSAO.m_SampleBias);
		sceneSettings.m_SSAOSettings.m_KernelSize = kernelSize.As<int>(DefaultSSAO.m_KernelSize);
		sceneSettings.m_SSAOSettings.m_BlurType = blurType.AsEnum<BlurType>(DefaultSSAO.m_BlurType);
		sceneSettings.m_SSAOSettings.m_BlurSize = blurSize.As<int>(DefaultSSAO.m_BlurSize);
		sceneSettings.m_SSAOSettings.m_Separation = separation.As<float>(DefaultSSAO.m_Separation);
		sceneSettings.m_SSAOSettings.m_BinsSize = binsSize.As<int>(DefaultSSAO.m_BinsSize);
		sceneSettings.m_SSAOSettings.m_Magnitude = magnitude.As<float>(DefaultSSAO.m_Magnitude);
		sceneSettings.m_SSAOSettings.m_Contrast = contrast.As<float>(DefaultSSAO.m_Contrast);
		sceneSettings.m_SSAOSettings.m_Dirty = true;
		return true;
	}

	void EditorResourceLoader::RemoveDeletedAssets()
	{
		if (IsCompilingAssets()) return;
		std::vector<UUID> removed{};
		std::vector<AssetLocation> removedLocations{};
		removed.reserve(m_ToRemoveAssets.size());

		for (auto& uuid : m_ToRemoveAssets)
		{
			EditorAssetDatabase::RemoveAsset(uuid, false);
			removed.push_back(uuid);
			AssetLocation location;
			EditorAssetDatabase::GetAssetLocation(uuid, location);
			removedLocations.push_back(std::move(location));
		}
		if (removed.empty()) return;
		CompileAssetDatabase(removed);
		RemovedAssetsPopup::AddRemovedAssets(std::move(removedLocations));
	}

	void EditorResourceLoader::AddTypeToLoadImmediately(const uint32_t type)
	{
		m_TypesToLoadImmediately.insert(type);
	}

	bool EditorResourceLoader::ShouldLoadImmediately(const uint32_t type) const
	{
		return m_TypesToLoadImmediately.find(type) != m_TypesToLoadImmediately.end();
	}

	AssetCompilerEventDispatcher& EditorResourceLoader::GetAssetCompilerEventDispatcher()
	{
		static AssetCompilerEventDispatcher dispatcher;
		return dispatcher;
	}

	std::filesystem::path EditorResourceLoader::GenerateCompiledResourcePath(const UUID uuid)
	{
		std::filesystem::path compiledPath = ProjectSpace::GetOpenProject()->CachePath();
		compiledPath.append("CompiledResources").append(std::to_string(uuid)).replace_extension(".gcag");
		return compiledPath;
	}

	void EditorResourceLoader::SetResourceNonCachable(const uint32_t type)
	{
		m_NonCachableResourceTypes.insert(type);
	}

	EditorResourceLoader::ResourceState EditorResourceLoader::GetResourceState(const UUID uuid) const
	{
		if (m_CompilingAssets.contains(uuid))
			return ResourceState::Compiling;
		if (m_CachingItems.contains(uuid))
			return ResourceState::Caching;
		if (m_LoadingResources.contains(uuid))
			return ResourceState::Loading;
		if (m_pResources->GetResource(uuid))
			return ResourceState::Loaded;
		return ResourceState::Unloaded;
	}

	void EditorResourceLoader::QueueLoad(UUID id, bool immediate)
	{
		/* Does a valid cache for the resource exist? */
		std::filesystem::path cachePath;
		std::filesystem::path assetPath;

		ResourceMeta meta;
		static const std::thread::id defaultThread = m_pApplication->GetEngine()->Threads().DefaultThreadID();
		const bool shouldLoadImmediately = immediate || std::this_thread::get_id() != defaultThread ||
			(EditorAssetDatabase::GetAssetMetadata(id, meta) && ShouldLoadImmediately(meta.Hash()));

		const bool hasValidCache = ResourceHasValidCache(id, cachePath, assetPath) && CheckCacheVersion(cachePath);
		if (hasValidCache)
		{
			/* Queue a cache load job instead */
			m_pDebug->LogInfo(std::format("EditorResourceLoader: Found valid cache for resource {}", uint64_t(id)));

			m_LoadingResources.insert(id);

			/* Load the cache */
			if (shouldLoadImmediately)
			{
				LoadCacheJob(id, cachePath);
				return;
			}

			ResourceLoaderJobPool->QueueSingleJob([this](UUID id, std::filesystem::path cachePath, Resource*)
				{ return LoadCacheJob(id, cachePath); }, id, cachePath, nullptr);

			return;
		}

		/* If we are already compiling the root asset we don't need to queue it again */
		if (m_AlreadyCompilingPaths.contains(assetPath)) return;
		m_AlreadyCompilingPaths.emplace(assetPath, 0);

		m_CompilingAssets.insert(id);

		/* Compile the asset */
		if (shouldLoadImmediately)
		{
			CompileJob(assetPath);
			return;
		}

		ResourceLoaderJobPool->QueueSingleJob([this](UUID, std::filesystem::path assetPath, Resource*)
			{ return CompileJob(assetPath); }, id, assetPath, nullptr);
	}

	void EditorResourceLoader::QueueUnload(UUID id)
	{
		m_ToUnload.insert(id);
	}

	void EditorResourceLoader::OnUpdate()
	{
		UpdateImportedResources();
		UpdateLoadedResources();
		UpdateCachedResources();
		HandleUnloading();
	}

	bool EditorResourceLoader::ResourceHasValidCache(UUID id, std::filesystem::path& cachePath, std::filesystem::path& assetPath) const
	{
		cachePath = GenerateCompiledResourcePath(id);

		AssetLocation location;
		GLORY_ASSERT(EditorAssetDatabase::GetAssetLocation(id, location), "Unknown resource ID");
		assetPath = EditorAssetDatabase::GetAbsoluteAssetPath(location.Path);

		if (!std::filesystem::exists(cachePath)) return false;

		const auto time = std::filesystem::last_write_time(cachePath);
		const uint64_t cacheWriteTime = std::chrono::duration_cast<std::chrono::seconds>(
		time.time_since_epoch()).count();

		const auto assetTime = std::filesystem::last_write_time(assetPath);
		const uint64_t assetWriteTime = std::chrono::duration_cast<std::chrono::seconds>(
			time.time_since_epoch()).count();

		return cacheWriteTime >= assetWriteTime;
	}

	bool EditorResourceLoader::CheckCacheVersion(std::filesystem::path& cachePath) const
	{
		Version version;
		{
			Utils::BinaryFileStream file{ cachePath, true, false };
			Utils::BinaryStream& stream = static_cast<Utils::BinaryStream&>(file);
			stream.Read(version);
		}

		return Version::Compare(CoreVersion, version) == 0;
	}

	bool EditorResourceLoader::CompileJob(const std::filesystem::path& assetPath)
	{
		/* Import the resource */
		ImportedResource resource = Importer::Import(assetPath);
		if (!resource)
		{
			/* There was an error during importing */
			m_pDebug->LogError(std::format("EditorResourceLoader: Failed to import asset at path {}", assetPath.string()));
			return false;
		}

		const uint32_t index = m_ImportWriteIndex.fetch_add(1) % ImportedResourcesRingBufferSize;
		m_ImportedResources[index] = std::move(resource);
		m_CurrentImportedResourceCount.fetch_add(1);

		return true;
	}

	bool EditorResourceLoader::CacheJob(UUID id, const std::filesystem::path& cachePath, Resource* pResource)
	{
		{
			Utils::BinaryFileStream cacheFile{ cachePath };
			AssetArchive archive{ &cacheFile, AssetArchiveFlags::WriteNew };
			archive.Serialize(pResource);
		}

		if (m_BuildingResourceCache)
			delete pResource;

		const uint32_t index = m_NextCachedResourceWriteIndex.fetch_add(1) % CachedResourcesRingBufferSize;
		m_CachedResources[index] = id;
		m_CurrentCachedResourceCount.fetch_add(1);

		return true;
	}

	bool EditorResourceLoader::LoadCacheJob(UUID id, const std::filesystem::path& cachePath)
	{
		{
			Utils::BinaryFileStream cacheFile{ cachePath, true, false };
			AssetArchive archive{ &cacheFile, AssetArchiveFlags::Read };
			archive.Deserialize(m_pDebug, m_pResources->GetResourceTypes());

			for (size_t i = 0; i < archive.Size(); ++i)
			{
				Resource* pNewResource = archive.Get(m_pDebug, i);
				const uint32_t nextResourceIndex = m_LoadedResourceWriteIndex.fetch_add(1) % LoadedResourcesRingBufferSize;
				m_LoadedResources[nextResourceIndex] = pNewResource;
				m_LoadedResourceIDs[nextResourceIndex] = id;
				m_CurrentLoadedResourceCount.fetch_add(1);
			}
		}
		return true;
	}

	static void ImportIfNew(ImportedResource& resource)
	{
		if (resource.IsNew())
		{
			EditorAssetDatabase::ImportAsset(resource.Path().string(), resource, resource.SubPath(), 0, false);
			return;
		}

		for (size_t i = 0; i < resource.ChildCount(); ++i)
			ImportIfNew(resource.Child(i));
	}

	void EditorResourceLoader::UpdateImportedResources()
	{
		const uint32_t size = m_CurrentImportedResourceCount.load();
		if (size == 0) return;
		/* Process 1 per frame to keep slowdown to an absolute minimum */
		const uint32_t actualSize = m_CurrentImportedResourceCount.fetch_add(-1);
		ImportedResource& resource = m_ImportedResources[m_CurrentImportedResourcesReadIndex];
		const std::filesystem::path path = resource.Path();
		++m_CurrentImportedResourcesReadIndex;
		m_CurrentImportedResourcesReadIndex = m_CurrentImportedResourcesReadIndex % ImportedResourcesRingBufferSize;
		if (!resource) return;
		ImportIfNew(resource);
		ProcessImportedResource(resource);

		/* Add missing resources to remove list */
		for (size_t i = 0; m_BuildingResourceCache && i < m_ToCheckRemovedResources.at(resource.Path()).size(); ++i)
		{
			for (const auto id : m_ToCheckRemovedResources.at(resource.Path()))
			{
				m_ToRemoveAssets.emplace_back(id);
			}
		}

		resource = nullptr;
		m_AlreadyCompilingPaths.erase(path);
	}

	void EditorResourceLoader::ProcessImportedResource(ImportedResource& resource)
	{
		for (size_t i = 0; i < resource.ChildCount(); ++i)
		{
			ProcessImportedResource(resource.Child(i));
		}

		Resource* pNewResource = *resource;
		const UUID resourceID = pNewResource->GetUUID();

		if (m_BuildingResourceCache)
			/* The resource exists so check if off the list */
			m_ToCheckRemovedResources.at(resource.Path()).erase(resourceID);

		if (!m_BuildingResourceCache && !m_pResources->AddResource(&pNewResource)) return;

		std::type_index type = typeid(Resource);
		pNewResource->GetType(0, type);

		const uint32_t hash = Hashing::Hash(type.name());

		if (!m_NonCachableResourceTypes.contains(hash))
		{
			const std::filesystem::path cachePath = GenerateCompiledResourcePath(resourceID);
			m_CachingItems.insert(resourceID);
			/* Dispatch a cache job */
			ResourceLoaderJobPool->QueueSingleJob([this](UUID id, std::filesystem::path cachePath, Resource* pResource)
				{ return CacheJob(id, cachePath, pResource); }, resourceID, cachePath, pNewResource);
		}

		m_pApplication->GetThumbnails().SetDirty(resourceID);
		GetAssetCompilerEventDispatcher().Dispatch({ resourceID });

		m_CompilingAssets.erase(resourceID);

		m_pDebug->LogInfo(std::format("EditorResourceLoader: Finished compiling resource {}", uint64_t(resourceID)));
	}

	void EditorResourceLoader::UpdateLoadedResources()
	{
		const uint32_t size = m_CurrentLoadedResourceCount.load();
		if (size == 0) return;

		/* We can process all of them since all we need to do is move them which is fast */
		m_CurrentLoadedResourceCount.fetch_sub(size);
		for (size_t i = 0; i < size; ++i)
		{
			Resource*& pResource = m_LoadedResources[m_CurrentLoadedResourceReadIndex];
			UUID& resourceID = m_LoadedResourceIDs[m_CurrentLoadedResourceReadIndex];
			++m_CurrentLoadedResourceReadIndex;
			m_CurrentLoadedResourceReadIndex = m_CurrentLoadedResourceReadIndex % LoadedResourcesRingBufferSize;
			if (pResource && !m_pResources->AddResource(&pResource))
				delete pResource;
			m_LoadingResources.erase(resourceID);
			pResource = nullptr;
		}
	}

	void EditorResourceLoader::UpdateCachedResources()
	{
		const uint32_t size = m_CurrentCachedResourceCount.load();
		if (size == 0) return;

		m_CurrentCachedResourceCount.fetch_sub(size);
		for (size_t i = 0; i < size; ++i)
		{
			UUID& resourceID = m_CachedResources[m_CurrentCachedResourceReadIndex];
			++m_CurrentCachedResourceReadIndex;
			m_CurrentCachedResourceReadIndex = m_CurrentCachedResourceReadIndex % CachedResourcesRingBufferSize;
			if (!resourceID) return;
			m_CachingItems.erase(resourceID);
			resourceID = 0;
		}
	}

	void EditorResourceLoader::HandleUnloading()
	{
		if (m_ToUnload.empty()) return;

		static constexpr size_t maxUnloadsInSingleFrame = 1000;
		std::array<UUID, maxUnloadsInSingleFrame> toUnload;
		size_t currentUnloadCount = 0;

		for (auto iter = m_ToUnload.begin(); iter != m_ToUnload.end(); ++iter)
		{
			if (currentUnloadCount >= maxUnloadsInSingleFrame) break;
			if (m_CachingItems.contains(*iter)) continue;
			toUnload[currentUnloadCount] = *iter;
			++currentUnloadCount;
		}

		for (size_t i = 0; i < currentUnloadCount; ++i)
		{
			m_pResources->UnloadResource(toUnload[i]);
			m_ToUnload.erase(toUnload[i]);
			m_pDebug->LogInfo(std::format("EditorResourceLoader: Resource {} was unloaded", uint64_t(toUnload[i])));
		}
	}
}
