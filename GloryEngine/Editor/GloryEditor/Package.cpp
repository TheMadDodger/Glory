#include "Package.h"
#include "ProjectSettings.h"
#include "SettingsEnums.h"
#include "EditorAssetDatabase.h"
#include "EditorSceneManager.h"

#include <Engine.h>
#include <AssetDatabase.h>
#include <GScene.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <AssetManager.h>

#include <filesystem>

namespace Glory::Editor
{
    void Package(Engine* pEngine)
    {
		std::filesystem::path packageRoot = ProjectSpace::GetOpenProject()->RootPath();
		packageRoot.append("Build");

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
			ScanSceneFileForAssets(sceneFile, assets);

			for (size_t j = 0; j < assets.size(); ++j)
			{
				std::vector<UUID>& assetScene = assetScenes[assets[j]];
				if(assetScene.empty() || std::find(assetScene.begin(), assetScene.end(), assetScene[i]) == assetScene.end())
					assetScene.push_back(scenes[i]);
			}
		}

		/* Find unique assets and assign them to a scene */
		std::map<UUID, std::vector<UUID>> assetsPerScene;

		struct Combo
		{
			std::vector<UUID> scenes;
			std::vector<UUID> assets;
		};
		std::vector<Combo> combos;
		for (auto itor = assetScenes.begin(); itor != assetScenes.end(); ++itor)
		{
			if (itor->second.size() == 1)
			{
				assetsPerScene[itor->second[0]].push_back(itor->first);
				continue;
			}
		}

		/* Check for user defined asset groups */
		/* @todo */

		/* Close all open scenes */
		/* @todo Make a backup of all open scenes to restore after packaging completes */
		EditorSceneManager::CloseAll();

		/* Open every scene and package them individually along with their assets */
		for (size_t i = 0; i < scenes.size(); ++i)
		{
			std::filesystem::path path = packageRoot;
			path.append("Scenes");
			if (!std::filesystem::exists(path))
				std::filesystem::create_directories(path);

			path.append(std::to_string(scenes[i])).replace_extension("gcag");

			BinaryFileStream file{ path };
			AssetArchive archive{ &file };

			EditorSceneManager::OpenScene(scenes[i], false);
			GScene* pScene = EditorSceneManager::GetActiveScene();
			archive.Serialize(pScene);
			EditorSceneManager::CloseAll();

			const std::vector<UUID>& assets = assetsPerScene.at(scenes[i]);
			for (size_t j = 0; j < assets.size(); ++j)
			{
				const UUID assetID = assets[j];
				Resource* pResource = pEngine->GetAssetManager().FindResource(assetID);
				archive.Serialize(pResource);
			}
		}

		/* Package other asset groups */
		/* @todo */

		/* Package asset database but only for used assets */
    }

	void ScanForAssets(Utils::NodeValueRef& node, std::vector<UUID>& assets)
	{
		static const uint32_t sceneHash = ResourceTypes::GetHash<GScene>();

		if (node.IsMap())
		{
			for (auto itor = node.Begin(); itor != node.End(); ++itor)
			{
				const std::string key = *itor;
				ScanForAssets(node[key], assets);
			}
		}
		else if (node.IsSequence())
		{
			for (size_t i = 0; i < node.Size(); ++i)
			{
				ScanForAssets(node[i], assets);
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
			}
			catch (const std::exception&)
			{
				return;
			}
		}
	}

	void ScanSceneFileForAssets(Utils::YAMLFileRef& file, std::vector<UUID>& assets)
	{
		Utils::NodeValueRef root = file.RootNodeRef().ValueRef();
		ScanForAssets(root, assets);
	}

    void PackageScene(GScene* pScene, const std::filesystem::path& path)
    {
        BinaryFileStream file{ path };
        AssetArchive archive{&file};
        archive.Serialize(pScene);
    }
}
