#pragma once
#include <Script.h>
#include <mono/jit/jit.h>
#include "AssemblyBinding.h"

namespace Glory
{
    class MonoScript : public Script
    {
    public:
        MonoScript();
        MonoScript(FileData* pFileData);
        virtual ~MonoScript();

        virtual void Invoke(Object* pObject, const std::string& method, void** args) override;

    private:
        virtual bool IsBehaviour() override;

    private:
        AssemblyClass* LoadClass(const std::string& lib, const std::string& namespaceName, const std::string& className);
        MonoObject* LoadObject(Object* pObject);

    private:
        friend class MonoScriptLoader;
        std::string m_NamespaceName;
        std::string m_ClassName;
        MonoClass* m_pMonoClass;
    };
}
