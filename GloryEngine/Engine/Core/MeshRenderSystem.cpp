#include "MeshRenderSystem.h"

#include "Engine.h"
#include "RendererModule.h"
#include "Debug.h"
#include "AssetDatabase.h"
#include "GScene.h"
#include "SceneManager.h"
#include "AssetLoader.h"
#include "Resources.h"
#include "AssetDatabase.h"
#include "MaterialInstanceData.h"
#include "MaterialManager.h"

#include <EntityRegistry.h>

namespace Glory
{
    void MeshRenderSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        Engine* pEngine = pRegistry->GetUserData<GScene*>()->Manager()->GetEngine();
        REQUIRE_MODULE(pEngine, RendererModule, );

        const UUID meshID = pComponent.m_Mesh.AssetUUID();
        const UUID materialID = pComponent.m_Material.AssetUUID();
        if (meshID && !pEngine->GetResources().Manager<MeshData>()->IsLoaded(meshID))
            pEngine->GetAssetLoader().RequestLoad(meshID);
        if (materialID && !pEngine->GetResources().Manager<MaterialData>()->IsLoaded(meshID))
            pEngine->GetAssetLoader().RequestLoad(materialID);
    }

    void MeshRenderSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        //REQUIRE_MODULE(Game::GetGame().GetEngine(), RendererModule, );

        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
        GScene* pScene = pRegistry->GetUserData<GScene*>();
        Engine* pEngine = pScene->Manager()->GetEngine();
        MaterialManager* pMaterials = &pEngine->GetMaterialManager();
        AssetDatabase* pAssetDB = &pEngine->GetAssetDatabase();
        LayerManager* pLayers = &pEngine->GetLayerManager();
        Debug* pDebug = &pEngine->GetDebug();

        const UUID meshID = pComponent.m_Mesh.AssetUUID();
        const UUID materialID = pComponent.m_Material.AssetUUID();

        if (!meshID || !materialID) return;

        Transform& transform = pRegistry->GetComponent<Transform>(entity);

        LayerMask mask;
        if (pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_Layer.Layer(pLayers) != nullptr ? layer.m_Layer.Layer(pLayers)->m_Mask : 0;
        }

        if (!pEngine->GetResources().Manager<MeshData>()->IsLoaded(meshID))
            return;

        if (!pAssetDB->AssetExists(pComponent.m_Material.AssetUUID()))
        {
            // TODO: Use some default material
            std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
            pDebug->LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
            return;
        }

        const UUID materialUUID = pComponent.m_Material.AssetUUID();
        MaterialData* pMaterial = pMaterials->GetMaterial(materialUUID);

        if (!materialLoaded)
        {
            // TODO: Use some default material
            return;
        }

        RenderData renderData;
        renderData.m_MeshID = meshID;
        renderData.m_MaterialID = materialID;
        renderData.m_World = transform.MatTransform;
        renderData.m_LayerMask = mask;
        renderData.m_ObjectID = pScene->GetEntityUUID(entity);
        renderData.m_SceneID = pScene->GetUUID();

        REQUIRE_MODULE_CALL(pEngine, RendererModule, Submit(renderData), );
    }
}
