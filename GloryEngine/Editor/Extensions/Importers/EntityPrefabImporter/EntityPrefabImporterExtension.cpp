#include "EntityPrefabImporterExtension.h"
#include "EntityPrefabImporter.h"

#include <Importer.h>

EXTENSION_CPP(EntityPrefabImporterExtension)

namespace Glory::Editor
{
	EntityPrefabImporter Importer;

	EntityPrefabImporterExtension::EntityPrefabImporterExtension()
	{
	}

	EntityPrefabImporterExtension::~EntityPrefabImporterExtension()
	{
	}

	void EntityPrefabImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}
