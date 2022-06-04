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
        virtual ~MonoScript();

        virtual void Invoke(Object* pObject, const std::string& method) override;

    private:


    private:
        AssemblyClass* LoadClass(const std::string& lib, const std::string& namespaceName, const std::string& className);
        MonoObject* LoadObject(Object* pObject);

    private:
        MonoClass* m_pMonoClass;
    };
}
