//#include "ModelRenderSystem.h"
//
//#include "Engine.h"
//#include "GScene.h"
//#include "SceneManager.h"
//#include "RendererModule.h"
//#include "Debug.h"
//#include "AssetManager.h"
//#include "AssetDatabase.h"
//
//#include <EntityRegistry.h>
//
//namespace Glory
//{
//    void ModelRenderSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ModelRenderer& pComponent)
//    {
//        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
//        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
//
//        GScene* pScene = pRegistry->GetUserData<GScene*>();
//        Engine* pEngine = pScene->Manager()->GetEngine();
//        AssetManager* pAssets = &pEngine->GetAssetManager();
//        AssetDatabase* pAssetDB = &pEngine->GetAssetDatabase();
//        LayerManager* pLayers = &pEngine->GetLayerManager();
//        Debug* pDebug = &pEngine->GetDebug();
//
//        Transform& transform = pRegistry->GetComponent<Transform>(entity);
//
//        LayerMask mask;
//        if (pRegistry->HasComponent<LayerComponent>(entity))
//        {
//            LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
//            mask = layer.m_Layer.Layer(pLayers) != nullptr ? layer.m_Layer.Layer(pLayers)->m_Mask : 0;
//        }
//
//        ModelData* pModelData = pAssets->GetOrLoadAsset<ModelData>(pComponent.m_Model.AssetUUID());
//        if (pModelData == nullptr) return;
//        for (size_t i = 0; i < pModelData->GetMeshCount(); i++)
//        {
//            MeshData* pMesh = pModelData->GetMesh(i);
//            if (!pMesh) return;
//            if (i >= pComponent.m_Materials.size() || !pAssetDB->AssetExists(pComponent.m_Materials[i].m_MaterialReference.AssetUUID()))
//            {
//                // TODO: Set some default material
//                std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
//                pDebug->LogOnce(key, "ModelRenderer: Missing Materials on ModelRenderer!", Debug::LogLevel::Warning);
//                continue;
//            }
//
//            UUID materialUUID = pComponent.m_Materials[i].m_MaterialReference.AssetUUID();
//            MaterialData* pMaterial = pAssets->GetOrLoadAsset<MaterialData>(materialUUID);
//
//            if (pMaterial == nullptr)
//            {
//                // TODO: Set some default material
//                return;
//            }
//
//            RenderData renderData;
//            renderData.m_pMesh = pMesh;
//            renderData.m_pMaterial = pMaterial;
//            renderData.m_World = transform.MatTransform;
//            renderData.m_LayerMask = mask;
//            renderData.m_ObjectID = entity;
//
//            REQUIRE_MODULE_CALL(pEngine, RendererModule, Submit(std::move(renderData)), );
//        }
//    }
//}