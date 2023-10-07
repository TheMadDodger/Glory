#include "ASSIMPImporter.h"
#include "ASSIMPImporterExtension.h"

#include <Importer.h>

EXTENSION_CPP(ASSIMPImporterExtension)

namespace Glory::Editor
{
	ASSIMPImporter Importer;

	ASSIMPImporterExtension::ASSIMPImporterExtension()
	{
	}

	ASSIMPImporterExtension::~ASSIMPImporterExtension()
	{
	}

	void ASSIMPImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}