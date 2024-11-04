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
        GLORY_API MonoScript(FileData* pFileData, std::string_view ns, std::string_view className);
        GLORY_API virtual ~MonoScript();

        GLORY_API void SetValue(MonoObject* pMonoObject, const std::string& name, void* value);
        GLORY_API void GetValue(MonoObject* pMonoObject, const std::string& name, void* value);

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;

    private:
        AssemblyClass* LoadClass(Assembly* pAssembly, const std::string& namespaceName, const std::string& className);

    private:
        friend class MonoScriptLoader;
        std::string m_NamespaceName;
        std::string m_ClassName;
        std::vector<ScriptProperty> m_ScriptProperties;
        std::vector<char> m_DefaultValues;
    };
}
