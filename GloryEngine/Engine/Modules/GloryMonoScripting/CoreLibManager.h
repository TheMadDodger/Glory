#pragma once
#include "IMonoLibManager.h"

namespace Glory
{
    class CoreLibManager : public IMonoLibManager
    {
    public:
        CoreLibManager();

        AssemblyBinding* GetAssemblyBinding() const;

    public:
        void Initialize(AssemblyBinding* pAssembly) override;
        void Cleanup() override;

    private:
        AssemblyBinding* m_pAssembly;
    };
}
