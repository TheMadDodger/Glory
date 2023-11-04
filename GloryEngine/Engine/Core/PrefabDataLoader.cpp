#include "PrefabDataLoader.h"

#include <yaml-cpp/yaml.h>
#include <NodeRef.h>
#include <fstream>

namespace Glory
{
    PrefabData* PrefabDataLoader::LoadResource(const std::string& path, const ImportSettings& importSettings)
    {
        PrefabData* pPrefab = new PrefabData();

        Utils::YAMLFileRef yamlFile{ path };
        const UUID originalUUID = yamlFile["OriginalUUID"].As<uint64_t>();
        const UUID transformUUID = yamlFile["TransformUUID"].As<uint64_t>();
        const bool activeSelf = yamlFile["ActiveSelf"].As<bool>();
        const std::string name = yamlFile["Name"].As<std::string>();
        Utils::NodeValueRef components = yamlFile["Components"];
        YAML::Emitter componentsWriter;
        componentsWriter << components.Node();
        PrefabNode rootNode = PrefabNode::Create(pPrefab, originalUUID, transformUUID, activeSelf, name, componentsWriter.c_str());

        Utils::NodeValueRef children = yamlFile["Children"];
        for (size_t i = 0; i < children.Size(); ++i)
        {
            Utils::NodeValueRef child = children[i];
            ReadChild(pPrefab, child, rootNode);
        }

        pPrefab->SetRootNode(std::move(rootNode));
        return pPrefab;
    }

    PrefabData* PrefabDataLoader::LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings)
    {
        return nullptr;
    }

    void PrefabDataLoader::SaveResource(const std::string& path, PrefabData* pResource)
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

    void PrefabDataLoader::WriteChild(YAML::Emitter& out, const PrefabNode& node)
    {
        out << YAML::Key << "OriginalUUID";
        out << YAML::Value << (uint64_t)node.OriginalUUID();
        out << YAML::Key << "TransformUUID";
        out << YAML::Value << (uint64_t)node.TransformUUID();
        out << YAML::Key << "ActiveSelf";
        out << YAML::Value << node.ActiveSelf();
        out << YAML::Key << "Name";
        out << YAML::Value << node.Name();

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

    void PrefabDataLoader::ReadChild(PrefabData* pPrefab, Utils::NodeValueRef yamlNode, PrefabNode& parent)
    {
        const UUID originalUUID = yamlNode["OriginalUUID"].As<uint64_t>();
        const UUID transformUUID = yamlNode["TransformUUID"].As<uint64_t>();
        const bool activeSelf = yamlNode["ActiveSelf"].As<bool>();
        const std::string name = yamlNode["Name"].As<std::string>();
        Utils::NodeValueRef components = yamlNode["Components"];
        YAML::Emitter componentsWriter;
        componentsWriter << components.Node();
        PrefabNode& childNode = parent.AddChild(pPrefab, originalUUID, transformUUID, activeSelf, name, componentsWriter.c_str());

        Utils::NodeValueRef children = yamlNode["Children"];
        for (size_t i = 0; i < children.Size(); ++i)
        {
            Utils::NodeValueRef child = children[i];
            ReadChild(pPrefab, child, childNode);
        }
    }
}
