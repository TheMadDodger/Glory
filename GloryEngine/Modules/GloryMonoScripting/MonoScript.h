#pragma once
#include <Script.h>
#include <Glory.h>

#include <mono/metadata/object-forward.h>
#include <mono/metadata/class.h>

namespace Glory
{
    class Assembly;
    struct AssemblyClass;

    class MonoScript : public Script
    {
    public:
        GLORY_API MonoScript();
        GLORY_API MonoScript(FileData* pFileData);
        virtual GLORY_API ~MonoScript();

        virtual void GLORY_API Invoke(UUID objectID, UUID sceneID, const std::string& method, void** args) override;
        virtual void GLORY_API InvokeSafe(UUID objectID, UUID sceneID, const std::string& method, std::vector<void*>& args) override;

        virtual void SetValue(UUID objectID, UUID sceneID, const std::string& name, void* value) override;
        virtual void GetValue(UUID objectID, UUID sceneID, const std::string& name, void* value) override;

        virtual void LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data) override;
        virtual void SetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node) override;
        virtual void GetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node) override;

    private:
        virtual bool IsBehaviour() override;

    private:
        AssemblyClass* LoadClass(Assembly* pAssembly, const std::string& namespaceName, const std::string& className);
        MonoObject* LoadObject(UUID objectID, UUID sceneID, MonoClass* pClass);

    private:
        friend class MonoScriptLoader;
        std::string m_NamespaceName;
        std::string m_ClassName;
    };
}
