#include "ProjectMigrations.h"
#include "EditorAssetDatabase.h"
#include "AssetCompiler.h"

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
        if (Version::Compare(version, { 0,1,1,0 }, true) < 0)
        {
            
        }

        /* Update version to current */
        editorVersion.SetString(GloryEditorVersion);
    }

    void Migrate_0_1_1_DefaultTextureAndMaterialProperties(ProjectSpace* pProject)
    {
        Debug::LogInfo("0.1.1> Migrating image datas to include default texture");

        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::vector<std::string_view> imageAssets;
        std::vector<std::string_view> materialAssets;
        std::vector<std::string_view> materialInstanceAssets;

        const uint32_t imageDataHash = ResourceType::GetHash<ImageData>();
        const uint32_t textureDataHash = ResourceType::GetHash<TextureData>();
        const uint32_t materialDataHash = ResourceType::GetHash<MaterialData>();
        const uint32_t materialInstanceDataHash = ResourceType::GetHash<MaterialInstanceData>();

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
            metaData["Hash"].SetUInt(ResourceType::GetHash<TextureData>());
            metaData["SerializedVersion"].SetUInt(0);

            std::stringstream debug;
            debug << "0.1.1> Migrated image data " << assetID << " to include default texture";
            Debug::LogInfo(debug.str());
        }

        Debug::LogInfo("0.1.1> Migrating material propeties");
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
                Debug::LogInfo(debug.str());
            }
            materialFile.Save();
        }

        Debug::LogInfo("0.1.1> Migrating material instance property overrides");
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
                Debug::LogInfo(debug.str());
            }
            materialFile.Save();
        }
    }

    void Migrate_0_1_1_AddGravityToPhysicsSettings(ProjectSpace* pProject)
    {
        Debug::LogInfo("0.1.1> Migrating PhysicsSettings to include Gravity property");
        std::filesystem::path physicsSettingsPath = pProject->RootPath();
        physicsSettingsPath.append("ProjectSettings").append("Physics.yaml");
        Utils::YAMLFileRef physicsSettings{ physicsSettingsPath };
        physicsSettings["Gravity"].Set(glm::vec3{0.0f, -9.81000042f, 0.0f});
        physicsSettings.Save();
    }
}
