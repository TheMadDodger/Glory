#include "STBImageImporterExtension.h"
#include "STBImageImporter.h"

#include <Importer.h>

EXTENSION_CPP(STBImageImporterExtension)

namespace Glory::Editor
{
	STBHDRImageImporter Importer;

	STBImageImporterExtension::STBImageImporterExtension()
	{
	}

	STBImageImporterExtension::~STBImageImporterExtension()
	{
	}

	void STBImageImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}
