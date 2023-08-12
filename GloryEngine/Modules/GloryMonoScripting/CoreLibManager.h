#pragma once
#include "IMonoLibManager.h"

namespace Glory
{
    class MonoManager;

    class CoreLibManager : public IMonoLibManager
    {
    public:
        CoreLibManager(MonoManager* pMonoManager);

        Assembly* GetAssemblyBinding() const;

    public:
        void Initialize(Assembly* pAssembly) override;
        void Cleanup() override;

    private:
        MonoManager* m_pMonoManager;
        Assembly* m_pAssembly;
    };
}
