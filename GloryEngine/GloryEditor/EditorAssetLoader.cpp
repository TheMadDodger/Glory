#include "EditorAssetLoader.h"
#include <yaml-cpp/yaml.h>


namespace Glory
{
	EditorAssetLoader::EditorAssetLoader() : m_pThread(nullptr) {}

	EditorAssetLoader::~EditorAssetLoader() {}

	void EditorAssetLoader::Initialize()
	{
		//m_pThread = ThreadManager::Run(std::bind(&EditorAssetLoader::Run, this));


	}

	void EditorAssetLoader::Run()
	{
		std::string rootPath = "./Assets";
		ProcessDirectory(rootPath);
	}

	void EditorAssetLoader::ProcessDirectory(const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				ProcessDirectory(entry.path().string());
				continue;
			}
			ProcessFile(entry.path());
		}
	}

	void EditorAssetLoader::ProcessFile(const std::filesystem::path& filePath)
	{
		//AssetDatabase* pInstance = GetInstance();

		auto ext = filePath.extension();
		std::filesystem::path metaExtension = std::filesystem::path(".meta");
		if (ext.compare(metaExtension) == 0) return; // No need to process meta files
		auto metaFilePath = filePath;
		metaFilePath = metaFilePath.replace_extension(metaExtension);

		if (std::filesystem::exists(metaFilePath))
		{
			// Both file and meta file exists we need to check if it also exists in the database!
			//Serialization::MetaData metaData = Serialization::MetaData::Read(metaFilePath.string());
			//if (pInstance->m_AssetPaths.find(metaData.m_GUID) != pInstance->m_AssetPaths.end()) return;
			//if (pInstance->GetAssetPath(metaData.m_GUID) != "") return;

			// Asset is missing from the database
			std::string pathToFile = filePath.string();
			int assetsIndex = pathToFile.find("Assets");
			std::string relativePathToFile = pathToFile.substr(assetsIndex + 6);
			//pInstance->m_AssetPaths[metaData.m_GUID] = relativePathToFile;
			return;
		}

		// Load the file
		//Content* pContent = ContentManager::GetInstance()->Load(filePath.string());
		//if (pContent == nullptr) return;
		//
		//const std::type_info& type = pContent->GetType();
		//size_t hashCode = SEObject::GetClassHash(type);

		// Make the path relative to the asset/resource path!
		std::string pathToFile = filePath.string();
		int assetsIndex = pathToFile.find("Assets");
		std::string relativePathToFile = pathToFile.substr(assetsIndex + 6);

		// Add the asset to the database and retreive the GUID
		//GUID newGUID = pInstance->AddAsset(type, filePath.filename().string(), relativePathToFile);
		//pContent->m_GUID = newGUID;
		//pContent->m_Name = filePath.filename().replace_extension().string();
		//
		//Serialization::MetaData metaData = Serialization::MetaData(hashCode, newGUID, metaFilePath.string());
		//metaData.Write();
		//
		//AssetManager::AddAsset(pContent);
	}
}
