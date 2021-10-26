#include "EditorAssetLoader.h"
#include <AssetDatabase.h>
#include <yaml-cpp/yaml.h>


namespace Glory::Editor
{
	EditorAssetLoader::EditorAssetLoader() : m_pThread(nullptr) {}

	EditorAssetLoader::~EditorAssetLoader() {}

	void EditorAssetLoader::Initialize()
	{
		//m_pThread = ThreadManager::Run(std::bind(&EditorAssetLoader::Run, this));


	}

	void EditorAssetLoader::LoadAll()
	{
		std::string rootPath = "./Assets";
		ProcessDirectory(rootPath, true);
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
		//AssetDatabase* pInstance = GetInstance();

		auto ext = filePath.extension();
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
		if (ext.compare(metaExtension) == 0) return; // No need to process meta files
		auto metaFilePath = filePath;
		metaFilePath = metaFilePath.replace_extension(metaExtension);

		if (std::filesystem::exists(metaFilePath))
		{
			// Both file and meta file exists we need to check if it also exists in the database!
			ResourceMeta meta(metaFilePath.string());
			uint64_t uuid = meta.ReadUUID();
			if (AssetDatabase::AssetExists(uuid)) return; // TODO: Check if the asset was moved if it does exist?

			// Asset is missing from the database
			std::string pathToFile = filePath.string();
			int assetsIndex = pathToFile.find("Assets");
			std::string relativePathToFile = pathToFile.substr(assetsIndex + 6);
			AssetDatabase::InsertAsset(relativePathToFile, meta);
			return;
		}

		// TODO: if the meta file does not exist but the asset exists in the database then we should regenerate the meta file

		// Generate a meta file
		std::string extension = ext.string();
		if (extension[0] == '.') extension = extension.substr(1);
		std::for_each(extension.begin(), extension.end(), [](char& c){ c = std::tolower(c); });
		ResourceType* pResourceType = ResourceType::GetResourceType(extension);
		size_t hash = 0;

		if (pResourceType != nullptr) hash = pResourceType->Hash();
		UUID generatedUUID;

		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);

		// Make the path relative to the asset/resource path!
		std::string pathToFile = filePath.string();
		int assetsIndex = pathToFile.find("Assets");
		std::string relativePathToFile = pathToFile.substr(assetsIndex + 6);

		ResourceMeta meta(metaFilePath.string(), generatedUUID, hash);
		meta.Write(pLoader, extension);
		meta.Read();

		AssetDatabase::InsertAsset(relativePathToFile, meta);
	}
}
