#include "MonoEntitySceneManager.h"

namespace Glory
{
    MonoObject* MonoEntitySceneManager::GetSceneObject(GScene* pScene)
    {
        return nullptr;
    }

    MonoSceneObjectManager* MonoEntitySceneManager::GetSceneObjectManager(GScene* pScene)
    {
        return new MonoEntityObjectManager(pScene);
    }

    void MonoEntitySceneManager::Initialize(AssemblyBinding* pAssembly)
    {
        /* TODO: Load mono class responsible for scene management */
    }

    void MonoEntitySceneManager::Cleanup()
    {
        /* TODO: Cleanup */
    }
}
