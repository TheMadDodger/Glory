#pragma once
#include "Components.h"

#include <ComponentManager.h>

namespace Glory
{
    class SceneManager;
    class Resources;
    class MaterialManager;
    class AssetDatabase;
    class LayerManager;
    class Debug;

    class MeshRenderManager : public Utils::ECS::ComponentManager<MeshRenderer>
    {
    public:
        GLORY_ENGINE_API MeshRenderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=100);
        GLORY_ENGINE_API virtual ~MeshRenderManager();

    public:
        GLORY_ENGINE_API void OnDirtyImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        GLORY_ENGINE_API void OnDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        GLORY_ENGINE_API void OnEnableDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        GLORY_ENGINE_API void OnDisableDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        GLORY_ENGINE_API void OnValidateImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        GLORY_ENGINE_API void GetReferencesImpl(std::vector<UUID>& references) const;
        GLORY_ENGINE_API void OnDeserialize(Utils::BinaryStream&) override;

    private:
        virtual void OnInitialize() override;

    private:
        friend class SceneManager;
        SceneManager* m_pSceneManager;
        Resources* m_pResources;
        MaterialManager* m_pMaterialManager;
        AssetDatabase* m_pAssetDatabase;
        LayerManager* m_pLayerManager;
        Debug* m_pDebug;
    };
}
