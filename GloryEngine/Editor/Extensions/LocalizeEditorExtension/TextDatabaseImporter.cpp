#include "TextDatabaseImporter.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>

#include <EditorApplication.h>

#include <NodeRef.h>
#include <Reflection.h>

namespace Glory::Editor
{
    std::string_view TextDatabaseImporter::Name() const
    {
        return "Text Database Importer";
    }

    bool TextDatabaseImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gtdb") == 0;
    }

    ImportedResource TextDatabaseImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        return nullptr;
    }

    bool TextDatabaseImporter::SaveResource(const std::filesystem::path& path, TextDatabase* pFSM) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        return false;
    }

    void TextDatabaseImporter::Initialize()
    {
    }
}
