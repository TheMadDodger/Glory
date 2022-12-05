#pragma once
#include <Script.h>
#include <mono/jit/jit.h>
#include <Glory.h>
#include "AssemblyBinding.h"

namespace Glory
{
    class MonoScript : public Script
    {
    public:
        GLORY_API MonoScript();
        GLORY_API MonoScript(FileData* pFileData);
        virtual GLORY_API ~MonoScript();

        virtual void GLORY_API Invoke(Object* pObject, const std::string& method, void** args) override;

        virtual void SetValue(Object* pObject, const std::string& name, void* value) override;
        virtual void GetValue(Object* pObject, const std::string& name, void* value) override;

        virtual void LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data) override;
        virtual void SetPropertyValues(Object* pObject, YAML::Node& node) override;
        virtual void GetPropertyValues(Object* pObject, YAML::Node& node) override;

    private:
        virtual bool IsBehaviour() override;

    private:
        AssemblyClass* LoadClass(const std::string& lib, const std::string& namespaceName, const std::string& className);
        MonoObject* LoadObject(Object* pObject, MonoClass* pClass);

    private:
        friend class MonoScriptLoader;
        std::string m_NamespaceName;
        std::string m_ClassName;
    };
}
