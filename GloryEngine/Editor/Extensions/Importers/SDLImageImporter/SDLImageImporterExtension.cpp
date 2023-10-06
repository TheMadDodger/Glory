#include "SDLImageImporterExtension.h"
#include "SDLImageImporter.h"

#include <Importer.h>

EXTENSION_CPP(SDLImageImporterExtension)

namespace Glory::Editor
{
	SDLImageImporter Importer;

	SDLImageImporterExtension::SDLImageImporterExtension()
	{
	}

	SDLImageImporterExtension::~SDLImageImporterExtension()
	{
	}

	void SDLImageImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}
