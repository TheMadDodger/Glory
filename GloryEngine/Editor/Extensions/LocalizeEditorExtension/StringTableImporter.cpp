#include "StringTableImporter.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>
#include <StringsOverrideTable.h>

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
        return extension.compare(".gtable") == 0 || extension.compare(".gotable") == 0;
    }

    void ReadFolder(Utils::NodeValueRef folder, StringTable* pStringTable, std::string fullKey="")
    {
        for (auto iter = folder.Begin(); iter != folder.End(); ++iter)
        {
            const std::string key = *iter;
            auto node = folder[key];
            std::string newFullKey = fullKey.empty() ? key : fullKey + "." + key;

            if (node.IsMap())
            {
                ReadFolder(node, pStringTable, newFullKey);
                continue;
            }
            std::string value = node.As<std::string>();
            pStringTable->AddString(std::move(newFullKey), std::move(value));
        }
    }

    ImportedResource StringTableImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        Utils::YAMLFileRef file{ path };
        auto root = file.RootNodeRef().ValueRef();
        if (!root.Exists() || !root.IsMap()) root.SetMap();

        if (path.extension().compare(".gotable") == 0)
        {
            /* Override table */
            const UUID baseTableID = root["BaseTable"].As<uint64_t>();
            std::string language = root["Language"].As<std::string>();

            StringsOverrideTable* pStringOverrideTable = new StringsOverrideTable(baseTableID, std::move(language));

            ReadFolder(root["Overrides"], pStringOverrideTable);
            return { path, pStringOverrideTable };
        }

        StringTable* pStringTable = new StringTable();
        ReadFolder(root, pStringTable);

        return { path, pStringTable };
    }

    void WriteStrings(Utils::NodeValueRef root, StringTable* pStringTable)
    {
        for (auto& iter = pStringTable->Begin(); iter != pStringTable->End(); ++iter)
        {
            std::vector<std::string_view> pathComponents;
            Reflect::Tokenize(iter->first, pathComponents, '.');

            std::filesystem::path path;
            for (size_t i = 0; i < pathComponents.size(); ++i)
            {
                path.append(pathComponents[i]);
            }

            root[path.string()].Set(iter->second);
        }
    }

    bool StringTableImporter::SaveResource(const std::filesystem::path& path, StringTable* pStringTable) const
    {
        Utils::YAMLFileRef file{ path };
        auto root = file.RootNodeRef().ValueRef();
        if (!root.Exists() || !root.IsMap()) root.SetMap();

        if (path.extension().compare(".gotable") == 0)
        {
            /* Override table */
            auto baseTableID = root["BaseTable"];
            auto language = root["Language"];

            StringsOverrideTable* pStringOverrideTable = static_cast<StringsOverrideTable*>(pStringTable);
            baseTableID.Set(uint64_t(pStringOverrideTable->BaseTableID()));
            language.Set(pStringOverrideTable->Language());

            WriteStrings(root["Overrides"], pStringOverrideTable);
            file.Save();
            return true;
        }

        WriteStrings(root, pStringTable);
        file.Save();
        return true;
    }

    void StringTableImporter::Initialize()
    {
    }
}
