#include "Package.h"
#include "ProjectSettings.h"
#include "SettingsEnums.h"
#include "EditorAssetDatabase.h"
#include "EditorShaderProcessor.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "AssetCompiler.h"
#include "EditableResource.h"

#include <ShaderSourceData.h>
#include <GScene.h>

#include <Engine.h>
#include <Debug.h>
#include <AssetDatabase.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <AssetManager.h>

#include <filesystem>
#include <tchar.h>

namespace Glory::Editor
{
    void Package(Engine* pEngine)
    {
		if (AssetCompiler::IsBusy())
		{
			pEngine->GetDebug().LogError("Cannot package while assets are compiling");
			return;
		}

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path packageRoot = pProject->RootPath();
		packageRoot.append("Build");

		std::filesystem::path dataPath = packageRoot;
		dataPath.append("Data");

		if (!std::filesystem::exists(dataPath))
			std::filesystem::create_directories(dataPath);

		EditorApplication::GetInstance()->OnBeginPackage(packageRoot);

		/* Compile project settings */
		std::filesystem::path settingsCompilePath = packageRoot;
		settingsCompilePath.append("Data/Sponza");
		ProjectSettings::Compile(settingsCompilePath);

        /* First we must gather what scenes will be packaged */
        ProjectSettings* packageSettings = ProjectSettings::Get("Packaging");
        Utils::YAMLFileRef& file = **packageSettings;
        auto scenePackageMode = file["Scenes/PackageScenesMode"];
        auto scenesToPackage = file["Scenes/List"];

		static const uint32_t sceneHash = ResourceTypes::GetHash<GScene>();
		static const uint32_t shaderSourceHash = ResourceTypes::GetHash<ShaderSourceData>();
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
		std::map<UUID, std::vector<UUID>> shadersPerScene;
		std::vector<UUID> sharedAssets;
		std::vector<UUID> sharedShaders;
		std::vector<UUID> shaders;
		for (auto itor = assetScenes.begin(); itor != assetScenes.end(); ++itor)
		{
			ResourceMeta meta;
			EditorAssetDatabase::GetAssetMetadata(itor->first, meta);

			/* Shaders are separate */
			if (meta.Hash() == shaderSourceHash)
			{
				shaders.push_back(itor->first);
				if (itor->second.size() == 1)
				{
					shadersPerScene[itor->second[0]].push_back(itor->first);
					continue;
				}
				sharedShaders.push_back(itor->first);
				continue;
			}

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
		/** @todo Make a backup of all open scenes to restore after packaging completes */
		EditorSceneManager::CloseAll();

		/* Compile shaders */
		/** @todo Actually compile the shaders for the chosen platform */
		for (size_t i = 0; i < shaders.size(); ++i)
		{
			const UUID uuid = shaders[i];
			Resource* pResource = pEngine->GetAssetManager().FindResource(uuid);
			if (!pResource) continue;
			ShaderSourceData* pShaderSource = static_cast<ShaderSourceData*>(pResource);
			FileData* pCompiledShader = pShaderSource->GetCompiledShader();
			if (pCompiledShader)
				delete pCompiledShader;

			FileData* pNewCompiled = pEngine->GetShaderManager().GetCompiledShaderFile(uuid);
			pCompiledShader = new FileData(pNewCompiled);
			pShaderSource->SetCompiledShader(pCompiledShader);
		}

		/* Open every scene and package them individually along with their assets */
		std::vector<UUID> usedAssets;
		std::vector<AssetLocation> assetLocations;
		for (size_t i = 0; i < scenes.size(); ++i)
		{
			std::filesystem::path relativeScenePath = "Data";
			relativeScenePath.append(std::to_string(scenes[i])).replace_extension("gcs");
			{
				std::filesystem::path path = packageRoot;
				path.append(relativeScenePath.string());
				BinaryFileStream sceneFile{ path };
				AssetArchive archive{ &sceneFile, true };

				EditorSceneManager::OpenScene(scenes[i], false);
				GScene* pScene = EditorSceneManager::GetActiveScene();
				archive.Serialize(pScene);
				EditorSceneManager::CloseAll();

				usedAssets.push_back(scenes[i]);
				assetLocations.push_back({ relativeScenePath.string(), "", 0 });
			}

			relativeScenePath.replace_extension("gcag");
			{
				std::filesystem::path path = packageRoot;
				path.append(relativeScenePath.string());
				BinaryFileStream sceneFile{ path };
				AssetArchive archive{ &sceneFile, true };

				const std::vector<UUID>& assets = assetsPerScene[scenes[i]];
				for (size_t j = 0; j < assets.size(); ++j)
				{
					const UUID assetID = assets[j];
					Resource* pResource = pEngine->GetAssetManager().FindResource(assetID);
					archive.Serialize(pResource);

					assetLocations.push_back({ relativeScenePath.string(), "", j });
					usedAssets.push_back(assetID);
				}
			}

			relativeScenePath.replace_extension("gcsp");
			{
				std::filesystem::path path = packageRoot;
				path.append(relativeScenePath.string());
				BinaryFileStream sceneFile{ path };
				AssetArchive archive{ &sceneFile, true };

				const std::vector<UUID>& shaders = shadersPerScene[scenes[i]];
				for (size_t j = 0; j < shaders.size(); ++j)
				{
					const UUID assetID = shaders[j];
					Resource* pResource = pEngine->GetAssetManager().FindResource(assetID);
					archive.Serialize(pResource);

					assetLocations.push_back({ relativeScenePath.string(), "", j });
					usedAssets.push_back(assetID);
				}
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

		if (!sharedShaders.empty())
		{
			/* Package other asset groups */
			std::filesystem::path relativePath = "Data";
			relativePath.append("Shared.gcsp");

			std::filesystem::path sharedShadersPath = packageRoot;
			sharedShadersPath.append(relativePath.string());
			BinaryFileStream sharedShadersFile{ sharedShadersPath };
			AssetArchive archive{ &sharedShadersFile };
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

		/* Copy modules and their resources and settings */
		std::filesystem::path modulesPath = packageRoot;
		modulesPath.append("Modules");
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

		/* Copy runtime and application */
		std::filesystem::path runtimePath = "./EditorAssets/Runtime";
		runtimePath.append("GloryRuntime.dll");
		std::filesystem::copy(runtimePath, packageRoot, std::filesystem::copy_options::overwrite_existing);
		runtimePath.replace_filename("GloryRuntimeApplication.dll");
		std::filesystem::copy(runtimePath, packageRoot, std::filesystem::copy_options::overwrite_existing);

		/* Compile exe */
		std::filesystem::path packagingCachePath = pProject->CachePath();
		packagingCachePath.append("Packaging");
		if (!std::filesystem::exists(packagingCachePath))
			std::filesystem::create_directories(packagingCachePath);

		std::filesystem::path appCPPPath = packagingCachePath;
		appCPPPath.append("Application.cpp");
		
		std::filesystem::path runtimeAPIHeaderPath = appCPPPath;
		runtimeAPIHeaderPath.replace_filename("RuntimeAPI.h");

		if (!std::filesystem::exists(appCPPPath))
		{
			runtimePath.replace_filename("Application.cpp");
			std::filesystem::copy(runtimePath, appCPPPath);
		}

		if (!std::filesystem::exists(runtimeAPIHeaderPath))
		{
			runtimePath.replace_filename("RuntimeAPI.h");
			std::filesystem::copy(runtimePath, runtimeAPIHeaderPath);
		}

		std::string entryScene;
		/** @todo Let the user decide what the entry scene is */
		if (!scenes.empty())
		{
			auto sceneZero = scenes[0];
			entryScene = std::to_string(sceneZero);
		}

		const std::string projectName = pProject->Name();
		/* Generate Configuration.h */
		std::filesystem::path configHeaderPath = packagingCachePath;
		configHeaderPath.append("Configuration.h");
		std::ofstream configHeaderStream(configHeaderPath);
		configHeaderStream << "#pragma once" << std::endl;
		configHeaderStream << "#include \"RuntimeAPI.h\"" << std::endl << std::endl;
		configHeaderStream << "namespace Config" << std::endl;
		configHeaderStream << "{" << std::endl;
		configHeaderStream << "	constexpr char* AppName = \"" << projectName << "\";" << std::endl;
		configHeaderStream << "	constexpr char* EntryScene = \"./Data/" << entryScene << ".gcs\";" << std::endl;
		configHeaderStream << "}" << std::endl << std::endl << std::endl;
		configHeaderStream << "inline void Exec()" << std::endl;
		configHeaderStream << "{" << std::endl;
		EditorApplication::GetInstance()->OnGenerateConfigExec(configHeaderStream);
		configHeaderStream << "}" << std::endl << std::endl;
		configHeaderStream.close();

		/* Generate premake file */
		std::filesystem::path luaPath = packagingCachePath;
		luaPath.append("premake5.lua");
		std::ofstream luaStream(luaPath);
		luaStream << "workspace \"" << projectName << "\"" << std::endl;
		luaStream << "	platforms { \"x64\" }" << std::endl;
		luaStream << "	configurations { \"Debug\", \"Release\" }" << std::endl;
		luaStream << "	flags { \"MultiProcessorCompile\" }" << std::endl;
		luaStream << "project \"" << projectName << "\"" << std::endl;
		luaStream << "	language \"C++\"" << std::endl;
		luaStream << "	cppdialect \"C++17\"" << std::endl;
		luaStream << "	staticruntime \"Off\"" << std::endl;
		luaStream << "	targetdir (\"bin\")" << std::endl;
		luaStream << "	objdir (\"bin/OBJ\")" << std::endl;
		luaStream << "	files " << std::endl;
		luaStream << "	{" << std::endl;
		luaStream << "		\"" << "*.h" << "\"," << std::endl;
		luaStream << "		\"" << "*.cpp" << "\"" << std::endl;
		luaStream << "	}" << std::endl;
		luaStream << "	filter \"" << "system:windows" << "\"" << std::endl;
		luaStream << "		defines \"" << "_CONSOLE" << "\"" << std::endl;
		luaStream << "		toolset \"v143\"" << std::endl;
		luaStream << "	filter \"" << "platforms:x64" << "\"" << std::endl;
		luaStream << "		architecture \"" << "x64" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Debug" << "\"" << std::endl;
		luaStream << "		kind \"" << "ConsoleApp" << "\"" << std::endl;
		luaStream << "		runtime \"" << "Debug" << "\"" << std::endl;
		luaStream << "		defines \"" << "_DEBUG" << "\"" << std::endl;
		luaStream << "		symbols \"" << "On" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Release" << "\"" << std::endl;
		luaStream << "		kind \"" << "WindowedApp" << "\"" << std::endl;
		luaStream << "		runtime \"" << "Release" << "\"" << std::endl;
		luaStream << "		defines \"" << "NDEBUG" << "\"" << std::endl;
		luaStream << "		optimize \"" << "On" << "\"" << std::endl;
		luaStream.close();

		/* Generate batch script */
		std::filesystem::path premakePath = std::filesystem::current_path();
		premakePath.append("premake").append("premake5.exe");
		packagingCachePath.append("premake5.lua");

		std::string cmd = "cd \"" + premakePath.parent_path().string() + "\" && " + "premake5.exe vs2019 --file=\"" + packagingCachePath.string() + "\"";
		system(cmd.c_str());
		packagingCachePath = packagingCachePath.parent_path();

#ifdef _DEBUG
		static const std::string config = "Debug";
#else
		static const std::string config = "Release";
#endif

		cmd = "cd \"" + packagingCachePath.string() + "\" && " + "msbuild /m /p:Configuration=" + config + " /p:Platform=x64 .";
		system(cmd.c_str());

		std::filesystem::path exePath = packagingCachePath;
		exePath.append("bin").append(projectName).replace_extension(".exe");
		std::filesystem::copy(exePath, packageRoot, std::filesystem::copy_options::overwrite_existing);

		EditorApplication::GetInstance()->OnEndPackage(packageRoot);
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
				AssetLocation location;
				EditorAssetDatabase::GetAssetLocation(assetID, location);
				if (meta.Hash() == sceneHash) return;

				assets.push_back(assetID);
				if (!location.SubresourcePath.empty())
				{
					/* Add the parent asset */
					const UUID parentID = EditorAssetDatabase::FindAssetUUID(location.Path);
					assets.push_back(parentID);
				}

				/* We also have to search this asset recursively for any referenced assets */
				EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(assetID);
				if (!pResource) return;
				YAMLResourceBase* pYAMLResource = dynamic_cast<YAMLResourceBase*>(pResource);
				if (!pYAMLResource) return;

				Utils::YAMLFileRef& assetFile = **pYAMLResource;
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
