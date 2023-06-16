#pragma once
#include "IMonoLibManager.h"

namespace Glory
{
    class MonoManager;

    class CoreLibManager : public IMonoLibManager
    {
    public:
        CoreLibManager(MonoManager* pMonoManager);

        AssemblyBinding* GetAssemblyBinding() const;

    public:
        void Initialize(AssemblyBinding* pAssembly) override;
        void Cleanup() override;

    private:
        MonoManager* m_pMonoManager;
        AssemblyBinding* m_pAssembly;
    };
}
