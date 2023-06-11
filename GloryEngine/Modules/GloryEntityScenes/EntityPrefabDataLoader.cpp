#include "EntityPrefabDataLoader.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Glory
{
    EntityPrefabData* EntityPrefabDataLoader::LoadResource(const std::string& path, const ImportSettings& importSettings)
    {
        return nullptr;
    }

    EntityPrefabData* EntityPrefabDataLoader::LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings)
    {
        return nullptr;
    }

    void EntityPrefabDataLoader::SaveResource(const std::string& path, EntityPrefabData* pResource)
    {
        YAML::Emitter out;
        const PrefabNode& node = pResource->RootNode();
        out << YAML::BeginMap;
        WriteChild(out, node);
        out << YAML::EndMap;

        std::ofstream outFile{path};
        outFile << out.c_str();
        outFile.close();
    }

    void EntityPrefabDataLoader::WriteChild(YAML::Emitter& out, const PrefabNode& node)
    {
        out << YAML::Key << "OriginalUUID";
        out << YAML::Value << (uint64_t)node.OriginalUUID();
        out << YAML::Key << "ActiveSelf";
        out << YAML::Value << node.ActiveSelf();

        out << YAML::Key << "Components";
        YAML::Node componentsNode = YAML::Load(node.SerializedComponents());
        out << YAML::Value << componentsNode;

        out << YAML::Key << "Children";
        out << YAML::Value << YAML::BeginSeq;
        for (size_t i = 0; i < node.ChildCount(); ++i)
        {
            const PrefabNode& childNode = node.ChildNode(i);
            out << YAML::BeginMap;
            WriteChild(out, childNode);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}
