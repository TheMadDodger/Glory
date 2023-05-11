#include "ProjectMigrations.h"
#include "EditorAssetDatabase.h"

#include <JSONRef.h>
#include <TextureData.h>

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

        /* Run migrations */
        if (Version::Compare(version, { 0,1,1,0 }, true) < 0)
        {
            Migrate_0_1_1_AddDefaultTextureInImageDatas(pProject);
        }

        /* Update version to current */
        editorVersion.SetString(GloryEditorVersion);
    }

    void Migrate_0_1_1_AddDefaultTextureInImageDatas(ProjectSpace* pProject)
    {
        JSONFileRef& projectFile = pProject->ProjectFile();
        JSONValueRef assets = projectFile["Assets"];

        std::vector<std::string_view> imageAssets;

        for (rapidjson::Value::ConstMemberIterator itor = assets.begin(); itor != assets.end(); ++itor)
        {
            JSONValueRef asset = assets[itor->name.GetString()];
            const uint32_t hash = asset["Metadata/Hash"].AsUInt();
            if (hash != ResourceType::GetHash<ImageData>()) continue;
            imageAssets.push_back(itor->name.GetString());
        }

        for (size_t i = 0; i < imageAssets.size(); i++)
        {
            const std::string_view assetID = imageAssets[i];
            JSONValueRef asset = assets[assetID];

            UUID newUUID{};
            std::to_string(newUUID);
            JSONValueRef defaultTextureAsset = assets[std::to_string(newUUID)];
            JSONValueRef location = defaultTextureAsset["Location"];
            JSONValueRef metaData = defaultTextureAsset["Metadata"];
            location["Path"].SetStringFromView(asset["Location/Path"].AsString());
            location["SubresourcePath"].SetStringFromView("Default");
            location["Index"].SetUInt(0);

            metaData["Extension"].SetStringFromView(asset["Metadata/Extension"].AsString());
            std::stringstream str;
            str << asset["Metadata/Name"].AsString() << "\\Default";
            metaData["Name"].SetStringFromView(asset["Metadata/Name"].AsString());
            metaData["UUID"].SetUInt64(newUUID);
            metaData["Hash"].SetUInt(ResourceType::GetHash<TextureData>());
            metaData["SerializedVersion"].SetUInt(0);
        }
    }
}
