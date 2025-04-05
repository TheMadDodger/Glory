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
        return "UI Document Importer";
    }

    bool FSMImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gfsm") == 0;
    }

    ImportedResource FSMImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        FSMData* pNewDocument = new FSMData();
		Utils::YAMLFileRef file{ path };
		auto node = file.RootNodeRef().ValueRef();

		Utils::NodeValueRef nodes = node["Nodes"];
		Utils::NodeValueRef transitions = node["Transitions"];
		
        /* @todo */

        return { path, pNewDocument };
    }

    bool FSMImporter::SaveResource(const std::filesystem::path& path, FSMData* pDocument) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        Utils::YAMLFileRef file{ path };

        auto node = file.RootNodeRef().ValueRef();

        if (!node.Exists() || !node.IsMap())
            node.Set(YAML::Node(YAML::NodeType::Map));

        Utils::NodeValueRef nodes = node["Nodes"];
        Utils::NodeValueRef transitions = node["Transitions"];
        nodes.Set(YAML::Node(YAML::NodeType::Map));
        transitions.Set(YAML::Node(YAML::NodeType::Map));

		/* @todo */

        file.Save();
        return true;
    }

    void FSMImporter::Initialize()
    {
    }
}
