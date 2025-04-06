#include "FSMImporter.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>
#include <FSMModule.h>

#include <EditorApplication.h>

#include <NodeRef.h>
#include <Reflection.h>

namespace Glory::Editor
{
    std::string_view FSMImporter::Name() const
    {
        return "FSM Importer";
    }

    bool FSMImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gfsm") == 0;
    }

    ImportedResource FSMImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        FSMData* pNewFSM = new FSMData();
		Utils::YAMLFileRef file{ path };
		auto node = file.RootNodeRef().ValueRef();

		Utils::NodeValueRef startNode = node["StartNode"];
		Utils::NodeValueRef nodes = node["Nodes"];
		Utils::NodeValueRef transitions = node["Transitions"];
        const UUID startNodeID = startNode.As<uint64_t>();

        for (auto iter = nodes.Begin(); iter != nodes.End(); ++iter)
        {
            const std::string idStr = *iter;
            auto node = nodes[idStr];
            const UUID id = node["ID"].As<uint64_t>();
            const std::string name = node["Name"].As<std::string>();
            const size_t index = pNewFSM->NodeCount();
            FSMNode& newFsmNode = pNewFSM->NewNode(name, id);
            if (id == startNodeID) pNewFSM->SetStartNodeIndex(index);
        }

        for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
        {
            const std::string idStr = *iter;
            auto transition = transitions[idStr];
            const UUID id = transition["ID"].As<uint64_t>();
            const std::string name = transition["Name"].As<std::string>();
            const UUID from = transition["From"].As<uint64_t>();
            const UUID to = transition["To"].As<uint64_t>();
            FSMTransition& fsmTransition = pNewFSM->NewTransition(name, from, to, id);
        }

        return { path, pNewFSM };
    }

    bool FSMImporter::SaveResource(const std::filesystem::path& path, FSMData* pFSM) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        Utils::YAMLFileRef file{ path };

        auto node = file.RootNodeRef().ValueRef();

        if (!node.Exists() || !node.IsMap())
            node.Set(YAML::Node(YAML::NodeType::Map));

        Utils::NodeValueRef startNode = node["StartNode"];
        Utils::NodeValueRef nodes = node["Nodes"];
        Utils::NodeValueRef transitions = node["Transitions"];
        nodes.Set(YAML::Node(YAML::NodeType::Map));
        transitions.Set(YAML::Node(YAML::NodeType::Map));

        startNode.Set((uint64_t)pFSM->StartNodeID());
        nodes.SetMap();
        transitions.SetMap();

        for (size_t i = 0; i < pFSM->NodeCount(); ++i)
        {
            const FSMNode& fsmNode = pFSM->Node(i);
            const std::string idStr = std::to_string(fsmNode.m_ID);
            auto node = nodes[idStr];
            node["ID"].Set((uint64_t)fsmNode.m_ID);
            node["Name"].Set(fsmNode.m_Name);
        }

        for (size_t i = 0; i < pFSM->TransitionCount(); ++i)
        {
            const FSMTransition& fsmTransition = pFSM->Transition(i);
            const std::string idStr = std::to_string(fsmTransition.m_ID);
            auto transition = transitions[idStr];
            transition["ID"].Set((uint64_t)fsmTransition.m_ID);
            transition["Name"].Set(fsmTransition.m_Name);
            transition["From"].Set((uint64_t)fsmTransition.m_FromNode);
            transition["To"].Set((uint64_t)fsmTransition.m_FromNode);
        }

        file.Save();
        return true;
    }

    void FSMImporter::Initialize()
    {
    }
}
