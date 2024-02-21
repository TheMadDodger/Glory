#include "Package.h"
#include "ProjectSettings.h"
#include "SettingsEnums.h"
#include "EditorAssetDatabase.h"
#include "EditorSceneManager.h"
#include "AssetCompiler.h"

#include <Engine.h>
#include <AssetDatabase.h>
#include <GScene.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <AssetManager.h>

#include <filesystem>

namespace Glory::Editor
{
	std::vector<std::string> YamlExtensions = {
		".gmat",
		".gminst",
		".gtex",
	};

    void Package(Engine* pEngine)
    {
		if (AssetCompiler::IsBusy()) return;

		std::filesystem::path packageRoot = ProjectSpace::GetOpenProject()->RootPath();
		packageRoot.append("Build");

		std::filesystem::path dataPath = packageRoot;
		dataPath.append("Data");

		if (!std::filesystem::exists(dataPath))
			std::filesystem::create_directories(dataPath);

        /* First we must gather what scenes will be packaged */
        ProjectSettings* packageSettings = ProjectSettings::Get("Packaging");
        Utils::YAMLFileRef& file = **packageSettings;
        auto scenePackageMode = file["Scenes/PackageScenesMode"];
        auto scenesToPackage = file["Scenes/List"];

		static const uint32_t sceneHash = ResourceTypes::GetHash<GScene>();
        const PackageScenes mode = scenePackageMode.AsEnum<PackageScenes>();
		std::vector<UUID> scenes;
		switch (mode)
		{
		case PackageScenes::All: {
			EditorAssetDatabase::GetAllAssetsOfType(sceneHash, scenes);
			break;
		}
		case PackageScenes::Opened: {
			break;
		}
		case PackageScenes::List: {
			for (size_t i = 0; i < scenesToPackage.Size(); ++i)
			{
				const UUID sceneID = scenesToPackage[i].As<uint64_t>();
				if (!sceneID) continue;
				scenes.push_back(sceneID);
			}
			break;
		}
		default:
			break;
		}

		if (scenes.empty()) return;

		/* Find all resources in use for each scene */
		std::map<UUID, std::vector<UUID>> assetScenes;
		for (size_t i = 0; i < scenes.size(); ++i)
		{
			AssetLocation location;
			EditorAssetDatabase::GetAssetLocation(scenes[i], location);
			std::filesystem::path path = pEngine->GetAssetDatabase().GetAssetPath();

			path.append(location.Path);

			if (!std::filesystem::exists(path))
				path = location.Path;

			Utils::YAMLFileRef sceneFile{ path };
			std::vector<UUID> assets;
			ScanSceneFileForAssets(pEngine, sceneFile, assets);

			for (size_t j = 0; j < assets.size(); ++j)
			{
				const UUID asset = assets[j];
				std::vector<UUID>& assetScene = assetScenes[asset];
				if(assetScene.empty() || std::find(assetScene.begin(), assetScene.end(), scenes[i]) == assetScene.end())
					assetScene.push_back(scenes[i]);
			}
		}

		/* Find unique assets and assign them to a scene */
		std::map<UUID, std::vector<UUID>> assetsPerScene;
		std::vector<UUID> sharedAssets;
		for (auto itor = assetScenes.begin(); itor != assetScenes.end(); ++itor)
		{
			if (itor->second.size() == 1)
			{
				assetsPerScene[itor->second[0]].push_back(itor->first);
				continue;
			}
			sharedAssets.push_back(itor->first);
		}

		/* Check for user defined asset groups */
		/* @todo */

		/* Close all open scenes */
		/* @todo Make a backup of all open scenes to restore after packaging completes */
		EditorSceneManager::CloseAll();

		/* Open every scene and package them individually along with their assets */
		std::vector<UUID> usedAssets;
		std::vector<AssetLocation> assetLocations;
		for (size_t i = 0; i < scenes.size(); ++i)
		{
			std::filesystem::path relativeScenePath = "Data";
			relativeScenePath.append(std::to_string(scenes[i])).replace_extension("gcag");

			std::filesystem::path path = packageRoot;
			path.append(relativeScenePath.string());
			BinaryFileStream sceneFile{ path };
			AssetArchive archive{ &sceneFile };

			EditorSceneManager::OpenScene(scenes[i], false);
			GScene* pScene = EditorSceneManager::GetActiveScene();
			archive.Serialize(pScene);
			EditorSceneManager::CloseAll();

			usedAssets.push_back(scenes[i]);
			assetLocations.push_back({ relativeScenePath.string(), "", 0 });

			const std::vector<UUID>& assets = assetsPerScene[scenes[i]];
			for (size_t j = 0; j < assets.size(); ++j)
			{
				const UUID assetID = assets[j];
				Resource* pResource = pEngine->GetAssetManager().FindResource(assetID);
				archive.Serialize(pResource);

				assetLocations.push_back({ relativeScenePath.string(), "", j + 1 });
				usedAssets.push_back(assetID);
			}
		}

		if(!sharedAssets.empty())
		{
			/* Package other asset groups */
			std::filesystem::path relativePath = "Data";
			relativePath.append("Shared.gcag");

			std::filesystem::path sharedAssetsPath = packageRoot;
			sharedAssetsPath.append(relativePath.string());
			BinaryFileStream sharedAssetsFile{ sharedAssetsPath };
			AssetArchive archive{ &sharedAssetsFile };
			for (size_t i = 0; i < sharedAssets.size(); ++i)
			{
				const UUID assetID = sharedAssets[i];
				Resource* pResource = pEngine->GetAssetManager().FindResource(assetID);
				archive.Serialize(pResource);

				assetLocations.push_back({ relativePath.string(), "", i });
				usedAssets.push_back(assetID);
			}
		}
		
		{
			/* Package asset database but only for used assets */
			std::filesystem::path databasePath = dataPath;
			databasePath.append("Assets.gcdb");
			BinaryFileStream dbFile{ databasePath };
			BinaryStream* stream = &dbFile;
			for (size_t i = 0; i < usedAssets.size(); ++i)
			{
				const UUID assetID = usedAssets[i];
				const AssetLocation& location = assetLocations[i];
				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(assetID, meta);
				stream->Write(meta.Name());
				stream->Write(meta.ID());
				stream->Write(meta.Hash());
				stream->Write(location.Path);
				stream->Write(location.Index);
			}
		}

		/* Compile shaders */

		/* Copy modules and their resources and settings */
		std::filesystem::path modulesPath = packageRoot;
		modulesPath.append("Engine");
		std::filesystem::path settingsPath = modulesPath;
		settingsPath.append("Config");

		if (!std::filesystem::exists(settingsPath))
			std::filesystem::create_directories(settingsPath);

		for (size_t i = 0; i < pEngine->ModulesCount(); i++)
		{
			Module* pModule = pEngine->GetModule(i);
			const ModuleMetaData& meta = pModule->GetMetaData();
			const std::string& moduleName = meta.Name();
			if (moduleName.empty() || meta.Path().empty()) continue;
			std::filesystem::path moduleRoot = meta.Path();
			moduleRoot = moduleRoot.parent_path();
			std::filesystem::path destination = modulesPath;
			destination.append(moduleRoot.filename().string());
			if (!std::filesystem::exists(destination))
				std::filesystem::create_directories(destination);

			std::filesystem::copy(moduleRoot, destination,
				std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

			std::filesystem::path editorPath = destination;
			editorPath.append("Editor");
			std::filesystem::remove_all(editorPath);

			std::filesystem::path assetsPath = destination;
			assetsPath.append("Assets");
			std::filesystem::remove_all(assetsPath);

			ModuleSettings& settings = pModule->Settings();
			std::filesystem::path moduleSettingsPath = settingsPath;
			moduleSettingsPath.append(moduleName + ".yaml");
			YAML::Emitter out;
			out << settings.Node();
			std::ofstream outFile(moduleSettingsPath);
			outFile << out.c_str();
			outFile.close();
		}
    }

	void ScanForAssets(Engine* pEngine, Utils::NodeValueRef& node, std::vector<UUID>& assets)
	{
		static const uint32_t sceneHash = ResourceTypes::GetHash<GScene>();

		if (node.IsMap())
		{
			for (auto itor = node.Begin(); itor != node.End(); ++itor)
			{
				const std::string key = *itor;
				ScanForAssets(pEngine, node[key], assets);
			}
		}
		else if (node.IsSequence())
		{
			for (size_t i = 0; i < node.Size(); ++i)
			{
				ScanForAssets(pEngine, node[i], assets);
			}
		}
		else if (node.IsScalar())
		{
			const std::string assetIDStr = node.As<std::string>();
			try
			{
				const UUID assetID = std::stoull(assetIDStr);
				if (!EditorAssetDatabase::AssetExists(assetID)) return;
				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(assetID, meta);
				if (meta.Hash() == sceneHash) return;
				assets.push_back(assetID);

				/* We also have to search this asset recursively for any referenced assets */
				std::filesystem::path path = pEngine->GetAssetDatabase().GetAssetPath();
				AssetLocation location;
				EditorAssetDatabase::GetAssetLocation(assetID, location);
				path.append(location.Path);

				auto itor = std::find(YamlExtensions.begin(), YamlExtensions.end(), path.extension().string());
				if (itor == YamlExtensions.end())
					return;

				Utils::YAMLFileRef assetFile{ path };
				ScanSceneFileForAssets(pEngine, assetFile, assets);
			}
			catch (const std::exception&)
			{
				return;
			}
		}
	}

	void ScanSceneFileForAssets(Engine* pEngine, Utils::YAMLFileRef& file, std::vector<UUID>& assets)
	{
		Utils::NodeValueRef root = file.RootNodeRef().ValueRef();
		ScanForAssets(pEngine, root, assets);
	}

    void PackageScene(GScene* pScene, const std::filesystem::path& path)
    {
        BinaryFileStream file{ path };
        AssetArchive archive{&file};
        archive.Serialize(pScene);
    }
}
