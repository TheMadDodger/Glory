#include "EditorAssetLoader.h"
#include <AssetDatabase.h>
#include "ProjectSpace.h"
#include <yaml-cpp/yaml.h>

namespace Glory::Editor
{
	EditorAssetLoader::EditorAssetLoader() : m_pThread(nullptr) {}

	EditorAssetLoader::~EditorAssetLoader() {}

	void EditorAssetLoader::LoadAssets()
	{
		LoadAll();
	}

	void EditorAssetLoader::Initialize()
	{
		//m_pThread = ThreadManager::Run(std::bind(&EditorAssetLoader::Run, this));


	}

	void EditorAssetLoader::LoadAll()
	{
		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets");

		ProcessDirectory(assetPath.string(), true);
		RemoveDeletedAssets();
	}

	void EditorAssetLoader::ProcessDirectory(const std::string& path, bool recursive)
	{
		if (!std::filesystem::is_directory(path)) return;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				if (recursive) ProcessDirectory(entry.path().string());
				continue;
			}
			ProcessFile(entry.path());
		}
	}

	void EditorAssetLoader::ProcessFile(const std::filesystem::path& filePath)
	{
		auto ext = filePath.extension();
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
		if (ext.compare(metaExtension) == 0) return; // No need to process meta files
		std::filesystem::path metaFilePath = filePath.string() + metaExtension.string();

		// Make the path relative to the asset/resource path!
		std::filesystem::path assetsPath = ProjectSpace::GetOpenProject()->RootPath();
		assetsPath.append("Assets");
		std::string relativePathToFile = filePath.lexically_relative(assetsPath).string();

		if (std::filesystem::exists(metaFilePath))
		{
			// Both file and meta file exists we need to check if it also exists in the database!
			ResourceMeta meta(metaFilePath.string(), ext.string());
			meta.Read();
			UUID uuid = meta.ID();
			if (AssetDatabase::AssetExists(uuid))
			{
				AssetLocation location;
				AssetDatabase::GetAssetLocation(uuid, location);
				if (relativePathToFile != location.m_Path)
				{
					AssetDatabase::UpdateAssetPath(uuid, relativePathToFile, metaFilePath.string());
				}
				return;
			}

			AssetDatabase::InsertAsset(relativePathToFile, meta);
			return;
		}

		std::string extension = ext.string();
		std::for_each(extension.begin(), extension.end(), [](char& c) { c = std::tolower(c); });
		ResourceType* pResourceType = ResourceType::GetResourceType(extension);
		size_t hash = 0;

		if (pResourceType != nullptr) hash = pResourceType->Hash();

		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);

		// If the meta file does not exist but the asset exists in the database then we should regenerate the meta file
		UUID uuid = AssetDatabase::GetAssetUUID(relativePathToFile);
		if (uuid != 0)
		{
			ResourceMeta meta;
			AssetDatabase::GetResourceMeta(uuid, meta);
			meta.Write(pLoader);
			return;
		}

		// Generate a meta file
		ResourceMeta meta(metaFilePath.string(), ext.string(), UUID(), hash);
		meta.Write(pLoader);
		meta.Read();

		AssetDatabase::InsertAsset(relativePathToFile, meta);
	}

	void EditorAssetLoader::RemoveDeletedAssets()
	{
		std::vector<UUID> toDeleteAssets;
		AssetDatabase::ForEachAssetLocation([&](UUID uuid, const AssetLocation& assetLocation)
		{
			std::filesystem::path path = Game::GetAssetPath();
			path.append(assetLocation.m_Path);
			if (std::filesystem::exists(path)) return;
			toDeleteAssets.push_back(uuid);
		});

		for (size_t i = 0; i < toDeleteAssets.size(); i++)
		{
			AssetDatabase::RemoveAsset(toDeleteAssets[i]);
		}
	}
}
