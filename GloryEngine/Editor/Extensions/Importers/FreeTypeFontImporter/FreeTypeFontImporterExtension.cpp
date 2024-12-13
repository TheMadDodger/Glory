#include "FreeTypeFontImporterExtension.h"
#include "FontImporter.h"

EXTENSION_CPP(FreeTypeFontImporter)

namespace Glory::Editor
{
	FontImporter Font_Importer;

	FreeTypeFontImporter::FreeTypeFontImporter()
	{
	}

	FreeTypeFontImporter::~FreeTypeFontImporter()
	{
	}

	void FreeTypeFontImporter::Initialize()
	{
		Importer::Register(&Font_Importer);
	}
}
