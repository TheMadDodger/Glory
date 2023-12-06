#include "MeshRenderSystem.h"

#include "Engine.h"
#include "RendererModule.h"
#include "Debug.h"
#include "AssetDatabase.h"
#include "GScene.h"
#include "SceneManager.h"
#include "MaterialManager.h"

#include <EntityRegistry.h>

namespace Glory
{
    void MeshRenderSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
        GScene* pScene = pRegistry->GetUserData<GScene*>();
        Engine* pEngine = pScene->Manager()->GetEngine();
        AssetManager* pAssets = &pEngine->GetAssetManager();
        MaterialManager* pMaterials = &pEngine->GetMaterialManager();
        AssetDatabase* pAssetDB = &pEngine->GetAssetDatabase();
        LayerManager* pLayers = &pEngine->GetLayerManager();
        Debug* pDebug = &pEngine->GetDebug();

        Transform& transform = pRegistry->GetComponent<Transform>(entity);

        LayerMask mask;
        if (pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_Layer.Layer(pLayers) != nullptr ? layer.m_Layer.Layer(pLayers)->m_Mask : 0;
        }

        MeshData* pMeshData = pAssets->GetOrLoadAsset<MeshData>(pComponent.m_Mesh.AssetUUID());
        if (pMeshData == nullptr) return;

        if (!pAssetDB->AssetExists(pComponent.m_Material.AssetUUID()))
        {
            // TODO: Set some default material
            std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
            pDebug->LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
            return;
        }

        const UUID materialUUID = pComponent.m_Material.AssetUUID();
        MaterialData* pMaterial = pMaterials->GetMaterial(materialUUID);

        if (pMaterial == nullptr)
        {
            // TODO: Set some default material
            return;
        }

        RenderData renderData;
        renderData.m_pMesh = pMeshData;
        renderData.m_pMaterial = pMaterial;
        renderData.m_World = transform.MatTransform;
        renderData.m_LayerMask = mask;
        renderData.m_ObjectID = pScene->GetEntityUUID(entity);
        renderData.m_SceneID = pScene->GetUUID();

        REQUIRE_MODULE_CALL(pEngine, RendererModule, Submit(renderData), );
    }
}
