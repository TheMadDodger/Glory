#include "EntityPrefabImporter.h"
#include "EditorSceneSerializer.h"

#include <fstream>
#include <EditorApplication.h>

namespace Glory::Editor
{
	EntityPrefabImporter::EntityPrefabImporter()
	{
	}

	EntityPrefabImporter::~EntityPrefabImporter()
	{
	}

	std::string_view EntityPrefabImporter::Name() const
	{
		return "Entity Prefab Importer";
	}

	bool EntityPrefabImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gentity") == 0;
	}

    ImportedResource EntityPrefabImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
        Utils::YAMLFileRef yamlFile{ path };
        PrefabData* pPrefab = new PrefabData();
		EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), pPrefab, yamlFile.RootNodeRef().ValueRef(), 0, "");
        return { path, pPrefab };
	}

	bool EntityPrefabImporter::SaveResource(const std::filesystem::path& path, PrefabData* pResource) const
	{
		Utils::YAMLFileRef yamlFile{ path };
        YAML::Emitter out;
        EditorSceneSerializer::SerializeScene(EditorApplication::GetInstance()->GetEngine(), pResource, yamlFile.RootNodeRef().ValueRef());
		yamlFile.Save();
        return true;
	}

	void EntityPrefabImporter::Initialize()
	{
	}

	void EntityPrefabImporter::Cleanup()
	{
	}
}
