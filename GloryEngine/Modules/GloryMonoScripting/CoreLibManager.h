#pragma once
#include "IMonoLibManager.h"
#include "MonoScriptManager.h"

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
        GLORY_API MonoObject* GetScriptDummy(MonoClass* pClass);
        GLORY_API MonoObject* CreateScript(size_t typeIndex, UUID sceneID, UUID objectID, UUID componentID);
        GLORY_API MonoObject* GetScript(UUID sceneID, UUID objectID, UUID componentID);
        GLORY_API const MonoScriptManager& ScriptManager() const;
        GLORY_API MonoScriptManager& ScriptManager();
        GLORY_API MonoObject* GetEngine();

    public:
        void Initialize(Engine* pEngine, Assembly* pAssembly) override;
        void Cleanup(Engine* pEngine) override;
        void Reset(Engine* pEngine) override;

        MonoObject* CreateAssetObject(UUID uuid, const std::string_view type);
        MonoObject* CreateSceneObject(UUID objectID, UUID sceneID);

    private:
        void CreateEngine(Engine* pEngine);

        void OnSceneDestroy(UUID sceneID);
        void OnSceneObjectDestroy(UUID objectID, UUID sceneID);

    private:
        MonoManager* m_pMonoManager;
        MonoScriptManager m_ScriptManager;
        Assembly* m_pAssembly;
        MonoObject* m_pEngineObject;
        MonoMethod* m_pEngineReset;
        uint32_t m_EngineGCHandle;

        UUID m_SceneClosingCallback;
        UUID m_SceneObjectDestroyedCallback;
    };
}
