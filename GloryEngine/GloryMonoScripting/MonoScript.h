#pragma once
#include <Script.h>
#include <mono/jit/jit.h>

namespace Glory
{
    class MonoScript : public Script
    {
    public:
        MonoScript();
        virtual ~MonoScript();

        virtual void Invoke(Object* pObject, const std::string& method) override;

    private:
        void LoadClass(const std::string& lib, const std::string& namespaceName);

    private:
        MonoClass* m_pMonoClass;
    };
}
