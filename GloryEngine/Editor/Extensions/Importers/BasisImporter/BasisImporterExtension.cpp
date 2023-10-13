#include "BasisImporterExtension.h"
#include "BasisImporter.h"

#include <Importer.h>

EXTENSION_CPP(BasisImporterExtension)

namespace Glory::Editor
{
	BasisImporter Importer;

	BasisImporterExtension::BasisImporterExtension()
	{
	}

	BasisImporterExtension::~BasisImporterExtension()
	{
	}

	void BasisImporterExtension::Initialize()
	{
		Importer::Register(&Importer);
	}
}
