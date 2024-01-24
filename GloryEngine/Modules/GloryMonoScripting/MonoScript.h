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
        GLORY_API MonoScript(const UUID uuid, const std::string& name);
        GLORY_API MonoScript(FileData* pFileData, std::string_view ns, std::string_view className);
        GLORY_API virtual ~MonoScript();

        GLORY_API void Invoke(UUID objectID, UUID sceneID, const std::string& method, void** args);
        GLORY_API void InvokeSafe(UUID objectID, UUID sceneID, const std::string& method, std::vector<void*>& args);

        GLORY_API void SetValue(UUID objectID, UUID sceneID, const std::string& name, void* value);
        GLORY_API void GetValue(UUID objectID, UUID sceneID, const std::string& name, void* value);

        GLORY_API void LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data);
        GLORY_API void SetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node);
        GLORY_API void GetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node);

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;

    private:
        GLORY_API bool IsBehaviour();

    private:
        AssemblyClass* LoadClass(Assembly* pAssembly, const std::string& namespaceName, const std::string& className);
        MonoObject* LoadObject(UUID objectID, UUID sceneID, MonoClass* pClass);

    private:
        friend class MonoScriptLoader;
        std::string m_NamespaceName;
        std::string m_ClassName;
    };
}
