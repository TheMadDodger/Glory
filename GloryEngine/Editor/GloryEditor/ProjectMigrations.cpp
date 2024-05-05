#include "ProjectMigrations.h"
#include "EditorAssetDatabase.h"
#include "AssetCompiler.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"

#include <JSONRef.h>
#include <TextureData.h>
#include <MaterialInstanceData.h>
#include <NodeRef.h>
#include <YAML_GLM.h>

namespace Glory::Editor
{
    void Migrate(ProjectSpace* pProject)
    {
        JSONFileRef& file = pProject->ProjectFile();
        JSONValueRef editorVersion = file["EditorVersion"];
        if (!editorVersion.Exists()) {
            /* Assume 0.1.0 */
            editorVersion.SetString("0.1.0");
        }

        const std::string_view editorVersionStr = editorVersion.AsString();
        const Version version = Version::Parse(editorVersionStr.data());

        /* Perform 0.1.1 migrations */
        if (Version::Compare(version, { 0,1,1,0 }, true) < 0)
        {
            Migrate_0_1_1_DefaultTextureAndMaterialProperties(pProject);
            Migrate_0_1_1_AddGravityToPhysicsSettings(pProject);
        }

        /* Perform 0.3.0 migrations */
        if (Version::Compare(version, { 0,3,0,0 }, true) < 0)
        {
            Migrate_0_3_0_ASSIMPAssets(pProject);
            Migrate_0_3_0_ConvertMaterialPropertiesToMap(pProject);
            Migrate_0_3_0_ModuleAssetIDS(pProject);
            Migrate_0_3_0_MaterialPipelines(pProject);
        }

        /* Update version to current */
        editorVersion.SetString(GloryEditorVersion);
    }

    void Migrate_0_1_1_DefaultTextureAndMaterialProperties(ProjectSpace* pProject)
    {
        EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("0.1.1> Migrating image datas to include default texture");

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::vector<std::string_view> imageAssets;
        std::vector<std::string_view> materialAssets;
        std::vector<std::string_view> materialInstanceAssets;

        const uint32_t imageDataHash = ResourceTypes::GetHash<ImageData>();
        const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();
        const uint32_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
        const uint32_t materialInstanceDataHash = ResourceTypes::GetHash<MaterialInstanceData>();

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();

            if (hash == imageDataHash)
            {
                imageAssets.push_back(itor->name.GetString());
            }
            else if (hash == materialDataHash)
            {
                materialAssets.push_back(itor->name.GetString());
            }
            else if (hash == materialInstanceDataHash)
            {
                materialInstanceAssets.push_back(itor->name.GetString());
            }
        }

        std::map<std::string_view, UUID> imageIDToTextureID;

        Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        for (size_t i = 0; i < imageAssets.size(); i++)
        {
            const std::string_view assetID = imageAssets[i];
            JSONValueRef asset = assets[assetID];

            UUID newUUID{};
            const std::string uuidString = std::to_string(newUUID);
            imageIDToTextureID.emplace(assetID, newUUID);
            JSONValueRef defaultTextureAsset = assets[uuidString];
            JSONValueRef location = defaultTextureAsset["Location"];
            JSONValueRef metaData = defaultTextureAsset["Metadata"];
            location["Path"].SetStringFromView(asset["Location/Path"].AsString());
            location["SubresourcePath"].SetStringFromView("Default");
            location["Index"].SetUInt(0);

            metaData["Extension"].SetStringFromView(asset["Metadata/Extension"].AsString());
            metaData["Name"].SetStringFromView(asset["Metadata/Name"].AsString());
            metaData["UUID"].SetUInt64(newUUID);
            metaData["Hash"].SetUInt(ResourceTypes::GetHash<TextureData>());
            metaData["SerializedVersion"].SetUInt(0);

            std::stringstream debug;
            debug << "0.1.1> Migrated image data " << assetID << " to include default texture";
            pEngine->GetDebug().LogInfo(debug.str());
        }

        pEngine->GetDebug().LogInfo("0.1.1> Migrating material propeties");
        for (size_t i = 0; i < materialAssets.size(); ++i)
        {
            const std::string_view materialAssetID = materialAssets[i];
            JSONValueRef asset = assets[materialAssetID];
            JSONValueRef location = asset["Location"];
            const std::string_view path = location["Path"].AsString();

            std::filesystem::path absolutePath = pProject->RootPath();
            absolutePath.append("Assets").append(path);

            if (!std::filesystem::exists(absolutePath)) continue;

            Utils::YAMLFileRef materialFile{ absolutePath };
            materialFile.Load();
            Utils::NodeValueRef properties = materialFile["Properties"];
            for (size_t i = 0; i < properties.Size(); ++i)
            {
                Utils::NodeValueRef property = properties[i];
                Utils::NodeValueRef propetyHash = property["TypeHash"];
                const uint32_t typeHash = propetyHash.As<uint32_t>();
                if (typeHash != imageDataHash) continue;
                propetyHash.Set(textureDataHash);
                Utils::NodeValueRef propertyValue = property["Value"];
                const UUID uuid = propertyValue.As<uint64_t>();
                auto itor = imageIDToTextureID.find(std::to_string(uuid));
                if (itor == imageIDToTextureID.end()) continue;
                const UUID newUUID = itor->second;
                propertyValue.Set((uint64_t)newUUID);

                std::stringstream debug;
                debug << "0.1.1> Migrated material " << materialAssetID << " propery " << property["DisplayName"].As<std::string>() << " to default texture " << newUUID;
                pEngine->GetDebug().LogInfo(debug.str());
            }
            materialFile.Save();
        }

        pEngine->GetDebug().LogInfo("0.1.1> Migrating material instance property overrides");
        for (size_t i = 0; i < materialInstanceAssets.size(); ++i)
        {
            const std::string_view materialAssetID = materialInstanceAssets[i];
            JSONValueRef asset = assets[materialAssetID];
            JSONValueRef location = asset["Location"];
            const std::string_view path = location["Path"].AsString();

            std::filesystem::path absolutePath = pProject->RootPath();
            absolutePath.append("Assets").append(path);

            if (!std::filesystem::exists(absolutePath)) continue;

            Utils::YAMLFileRef materialFile{ absolutePath };
            materialFile.Load();
            Utils::NodeValueRef overrides = materialFile["Overrides"];
            for (size_t i = 0; i < overrides.Size(); ++i)
            {
                Utils::NodeValueRef override = overrides[i];
                Utils::NodeValueRef overrideValue = override["Value"];
                if (!overrideValue.IsScalar()) continue;
                const UUID uuid = overrideValue.As<uint64_t>();
                auto itor = imageIDToTextureID.find(std::to_string(uuid));
                if (itor == imageIDToTextureID.end()) continue;
                const UUID newUUID = itor->second;
                overrideValue.Set((uint64_t)newUUID);

                std::stringstream debug;
                debug << "0.1.1> Migrated material instance " << materialAssetID << " propery " << override["DisplayName"].As<std::string>() << " to default texture " << newUUID;
                pEngine->GetDebug().LogInfo(debug.str());
            }
            materialFile.Save();
        }
    }

    void Migrate_0_1_1_AddGravityToPhysicsSettings(ProjectSpace* pProject)
    {
        EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("0.1.1> Migrating PhysicsSettings to include Gravity property");
        std::filesystem::path physicsSettingsPath = pProject->RootPath();
        physicsSettingsPath.append("ProjectSettings").append("Physics.yaml");
        Utils::YAMLFileRef physicsSettings{ physicsSettingsPath };
        physicsSettings["Gravity"].Set(glm::vec3{0.0f, -9.81000042f, 0.0f});
        physicsSettings.Save();
    }

    void Migrate_0_3_0_ConvertMaterialPropertiesToMap(ProjectSpace* pProject)
    {
        EditorApplication* pApplication = EditorApplication::GetInstance();

        pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrating material property arrays to maps");

        const uint32_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
        const uint32_t materialInstanceDataHash = ResourceTypes::GetHash<MaterialInstanceData>();

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::vector<std::string_view> materialAssets;
        std::vector<std::string_view> materialInstanceAssets;

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();

            if (hash == materialDataHash)
            {
                materialAssets.push_back(itor->name.GetString());
            }
            else if (hash == materialInstanceDataHash)
            {
                materialInstanceAssets.push_back(itor->name.GetString());
            }
        }

        for (auto id : materialAssets)
        {
            JSONValueRef asset = assets[id];
            const UUID uuid = asset["Metadata/UUID"].AsUInt64();
            EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(uuid);
            if (!pResource)
            {
                pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Failed to migrate a material");
                continue;
            }
            YAMLResource<MaterialData>* pMaterial = static_cast<YAMLResource<MaterialData>*>(pResource);
            Utils::YAMLFileRef& file = **pMaterial;
            if (file["Properties"].IsMap()) continue;

            YAML::Node propertiesNode = file["Properties"].Node();
            file["OldProperties"].Set(propertiesNode);
            file.RootNodeRef().ValueRef().Remove("Properties");

            for (size_t i = 0; i < file["OldProperties"].Size(); ++i)
            {
                const std::string propId = file["OldProperties"][i]["ShaderName"].As<std::string>();
                const uint32_t hash = file["OldProperties"][i]["TypeHash"].As<uint32_t>();
                YAML::Node value = file["OldProperties"][i]["Value"].Node();

                auto newProp = file["Properties"][propId];
                newProp["DisplayName"].Set(propId);
                newProp["TypeHash"].Set(hash);
                newProp["Value"].Set(value);
            }
            file.RootNodeRef().ValueRef().Remove("OldProperties");
            EditorAssetDatabase::SetAssetDirty(uuid);
            pMaterial->SetDirty(true);
            pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrated material properties for " + std::to_string(uuid));
        }

        for (auto id : materialInstanceAssets)
        {
            JSONValueRef asset = assets[id];
            const UUID uuid = asset["Metadata/UUID"].AsUInt64();
            EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(uuid);
            if (!pResource)
            {
                pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Failed to migrate a material");
                continue;
            }
            YAMLResource<MaterialInstanceData>* pMaterial = static_cast<YAMLResource<MaterialInstanceData>*>(pResource);
            Utils::YAMLFileRef& file = **pMaterial;
            if (file["Overrides"].IsMap()) continue;

            YAML::Node overridesNode = file["Overrides"].Node();
            file["OldOverrides"].Set(overridesNode);
            file.RootNodeRef().ValueRef().Remove("Overrides");

            for (size_t i = 0; i < file["OldOverrides"].Size(); ++i)
            {
                const std::string propId = file["OldOverrides"][i]["DisplayName"].As<std::string>();
                YAML::Node value = file["OldOverrides"][i]["Value"].Node();

                auto newProp = file["Overrides"][propId];
                newProp["Enable"].Set(true);
                newProp["Value"].Set(value);
            }
            file.RootNodeRef().ValueRef().Remove("OldOverrides");
            EditorAssetDatabase::SetAssetDirty(uuid);
            pMaterial->SetDirty(true);
            pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrated material instance overrides for " + std::to_string(uuid));
        }
    }

    void Migrate_0_3_0_ASSIMPAssets(ProjectSpace* pProject)
    {
        EditorApplication* pApplication = EditorApplication::GetInstance();

        pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrating ASSIMP module assets to new location");

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();
            const std::string_view pathStr = asset["Location/Path"].AsString();
            if (pathStr._Starts_with(".\\Modules\\GloryASSIMPModelLoader\\Assets\\Models\\"))
            {
                std::filesystem::path path = pathStr;
                path = path.lexically_relative(".\\Modules\\GloryASSIMPModelLoader\\Assets\\Models\\");
                std::filesystem::path newPath = ".\\Modules\\GloryClusteredRenderer\\Assets\\Models\\";
                newPath.append(path.string());
                asset["Location/Path"].SetString(newPath.string());

                std::stringstream str;
                str << "0.3.0> Moved " << pathStr << " to " << newPath.string();
                pApplication->GetEngine()->GetDebug().LogInfo(str.str());
            }
        }
    }

    void Migrate_0_3_0_ModuleAssetIDS(ProjectSpace* pProject)
    {
        EditorApplication* pApplication = EditorApplication::GetInstance();

        pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrating module asset IDs to new defaults");

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::map<UUID, UUID> moveMap;
        std::vector<std::string> materials;

        const uint32_t materialDataHash = ResourceTypes::GetHash<MaterialData>();

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();
            if (hash == materialDataHash)
                materials.push_back(itor->name.GetString());

            const std::string_view pathStr = asset["Location/Path"].AsString();
            if (pathStr._Starts_with(".\\Modules\\"))
            {
                std::filesystem::path path = pathStr;
                while (!path.empty() && path.filename().compare("Assets") != 0)
                    path = path.parent_path();

                const uint32_t hash = asset["Metadata/Hash"].AsUInt();

                if (path.empty()) continue;
                path.append("Assets.yaml");
                if (!std::filesystem::exists(path)) continue;

                Utils::YAMLFileRef file{ path };
                auto root = file.RootNodeRef().ValueRef();
                for (size_t i = 0; i < root.Size(); ++i)
                {
                    const std::string otherPath = root[i]["Path"].As<std::string>();
                    const uint64_t oldUUID = asset["Metadata/UUID"].AsUInt64();
                    const uint64_t newUUID = root[i]["ID"].As<uint64_t>();
                    if (pathStr.find(otherPath) == std::string::npos) continue;
                    asset["Metadata/UUID"].SetUInt64(newUUID);

                    std::stringstream str;
                    str << "0.3.0> Updated ID for " << pathStr << " to " << newUUID;
                    pApplication->GetEngine()->GetDebug().LogInfo(str.str());
                    moveMap.emplace(oldUUID, newUUID);
                    break;
                }
            }
        }

        for (auto itor = moveMap.begin(); itor != moveMap.end(); ++itor)
        {
            assets[std::to_string(itor->second)].Value() = assets[std::to_string(itor->first)].Value();
            assets.Remove(std::to_string(itor->first));
        }

        for (size_t i = 0; i < materials.size(); ++i)
        {
            JSONValueRef asset = assets[materials[i]];
            JSONValueRef location = asset["Location"];
            if (!location["SubresourcePath"].AsString().empty()) continue;
            const UUID uuid = asset["Metadata/UUID"].AsUInt64();
            EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(uuid);
            if (!pResource)
            {
                pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Failed to migrate a material");
                continue;
            }

            YAMLResource<MaterialData>* pMaterial = static_cast<YAMLResource<MaterialData>*>(pResource);
            Utils::YAMLFileRef& file = **pMaterial;

            auto shaders = file["Shaders"];
            for (size_t i = 0; i < shaders.Size(); ++i)
            {
                const UUID oldUUID = shaders[i]["UUID"].As<uint64_t>();
                const auto itor = moveMap.find(oldUUID);
                if (itor == moveMap.end()) continue;
                shaders[i]["UUID"].Set(uint64_t(itor->second));
                EditorAssetDatabase::SetAssetDirty(uuid);
            }
        }
    }

    void Migrate_0_3_0_MaterialPipelines(ProjectSpace* pProject)
    {
        EditorApplication* pApplication = EditorApplication::GetInstance();

        pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Migrating materials to use pipelines");

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::map<UUID, UUID> moveMap;
        std::vector<std::string> materials;

        const uint32_t materialDataHash = ResourceTypes::GetHash<MaterialData>();

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();
            if (hash != materialDataHash) continue;
            materials.push_back(itor->name.GetString());

            JSONValueRef location = asset["Location"];
            if (!location["SubresourcePath"].AsString().empty()) continue;
            const UUID uuid = asset["Metadata/UUID"].AsUInt64();
            EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(uuid);
            if (!pResource)
            {
                pApplication->GetEngine()->GetDebug().LogInfo("0.3.0> Failed to migrate a material");
                continue;
            }

            YAMLResource<MaterialData>* pMaterial = static_cast<YAMLResource<MaterialData>*>(pResource);
            Utils::YAMLFileRef& file = **pMaterial;

            auto shaders = file["Shaders"];
            auto pipeline = file["Pipeline"];
            std::string shadersStr;
            for (size_t i = 0; i < shaders.Size(); ++i)
            {
                const UUID shaderID = shaders[i]["UUID"].As<uint64_t>();
                shadersStr += std::to_string(shaderID);
            }

            if (shadersStr == "12" || shadersStr == "21")
            {
                /* Use phong, which is ID 9 */
                pipeline.Set(uint64_t(9));
            }
            else if (shadersStr == "34" || shadersStr == "43")
            {
                /* Use phong textured, which is ID 10 */
                pipeline.Set(uint64_t(10));
            }
            else
            {
                /* Fall back to phong */
                pipeline.Set(uint64_t(9));
            }

            file.RootNodeRef().ValueRef().Remove("Shaders");
            EditorAssetDatabase::SetAssetDirty(uuid);
        }
    }
}
