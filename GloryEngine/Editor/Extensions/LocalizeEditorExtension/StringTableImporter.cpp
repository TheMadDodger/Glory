#include "StringTableImporter.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>

#include <EditorApplication.h>

#include <NodeRef.h>
#include <Reflection.h>

namespace Glory::Editor
{
    std::string_view StringTableImporter::Name() const
    {
        return "String Table Importer";
    }

    bool StringTableImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gtable") == 0;
    }

    ImportedResource StringTableImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        StringTable* pStringTable = new StringTable();

        Utils::YAMLFileRef file{ path };
        auto root = file.RootNodeRef().ValueRef();
        if (!root.Exists() || !root.IsMap()) root.SetMap();

        for (auto iter = root.Begin(); iter != root.End(); ++iter)
        {
            std::string key = *iter;
            std::string value = root[key].As<std::string>();
            pStringTable->AddString(std::move(key), std::move(value));
        }

        return { path, pStringTable };
    }

    bool StringTableImporter::SaveResource(const std::filesystem::path& path, StringTable* pStringTable) const
    {
        Utils::YAMLFileRef file{ path };
        auto root = file.RootNodeRef().ValueRef();
        if (!root.Exists() || !root.IsMap()) root.SetMap();

        for (auto& iter = pStringTable->Begin(); iter != pStringTable->End(); ++iter)
        {
            root[iter->first].Set(iter->second);
        }

        file.Save();
        return true;
    }

    void StringTableImporter::Initialize()
    {
    }
}
