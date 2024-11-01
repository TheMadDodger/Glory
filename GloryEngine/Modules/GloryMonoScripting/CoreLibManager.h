#pragma once
#include "IMonoLibManager.h"

#include <UUID.h>
#include <Glory.h>
#include <mono/metadata/object-forward.h>

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

        MonoObject* CreateAssetObject(UUID ui, const std::string_view type);

    private:
        void CreateEngine(Engine* pEngine);

    private:
        MonoManager* m_pMonoManager;
        Assembly* m_pAssembly;
        MonoObject* m_pEngineObject;
        MonoMethod* m_pEngineReset;
    };
}
