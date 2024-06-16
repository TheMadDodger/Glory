#include "GenericAudioImporterExtension.h"
#include "GenericAudioImporter.h"

#include <Importer.h>

EXTENSION_CPP(GenericAudioImporterExtension)

namespace Glory::Editor
{
	GenericAudioImporter Importer;

	GenericAudioImporterExtension::GenericAudioImporterExtension()
	{
	}

	GenericAudioImporterExtension::~GenericAudioImporterExtension()
	{
	}

	void GenericAudioImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}
