#include "ASSIMPImporter.h"
#include "ASSIMPImporterExtension.h"

#include <Importer.h>
#include <EditorApplication.h>
#include <EditorSceneManager.h>

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
		Reflect::SetReflectInstance(&EditorApplication::GetInstance()->GetEngine()->Reflection());
		EditorApplication::GetInstance()->GetSceneManager().ComponentTypesInstance();
	}
}