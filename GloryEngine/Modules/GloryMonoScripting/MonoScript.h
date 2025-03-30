#pragma once
#include "ScriptProperty.h"

#include <FileData.h>
#include <Glory.h>

#include <mono/metadata/object-forward.h>
#include <mono/metadata/class.h>

namespace YAML
{
    class Node;
}

namespace Glory
{
    class Assembly;
    struct AssemblyClass;

    class MonoScript : public FileData
    {
    public:
        GLORY_API MonoScript();
        GLORY_API MonoScript(FileData* pFileData, std::vector<std::string>&& classes);
        GLORY_API MonoScript(std::vector<char>&& data);
        GLORY_API virtual ~MonoScript();

        GLORY_API void Serialize(BinaryStream& container) const override {};
        GLORY_API void Deserialize(BinaryStream& container) override {};

        GLORY_API bool HasClass(std::string_view name) const;
        GLORY_API std::string_view ClassName(size_t index) const;
        GLORY_API size_t ClassCount() const;

    private:
        virtual void References(Engine*, std::vector<UUID>&) const override {}

    private:
        friend class MonoScriptLoader;
        std::vector<std::string> m_Classes;
        std::vector<ScriptProperty> m_ScriptProperties;
        std::vector<char> m_DefaultValues;
    };
}
