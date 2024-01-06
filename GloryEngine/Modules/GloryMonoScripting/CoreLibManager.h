#pragma once
#include "IMonoLibManager.h"

#include <Glory.h>

namespace Glory
{
    class MonoManager;

    class CoreLibManager : public IMonoLibManager
    {
    public:
        GLORY_API CoreLibManager(MonoManager* pMonoManager);
        GLORY_API Assembly* GetAssemblyBinding() const;

    public:
        void Initialize(Engine* pEngine, Assembly* pAssembly) override;
        void Cleanup() override;

    private:
        MonoManager* m_pMonoManager;
        Assembly* m_pAssembly;
    };
}
