#include "MeshRenderManager.h"

//#include "TransformManager.h"

#include "Renderer.h"
#include "Debug.h"
#include "Resources.h"
#include "AssetDatabase.h"
#include "GScene.h"
#include "SceneManager.h"
#include "MaterialManager.h"

#include <EntityRegistry.h>

namespace Glory
{
    MeshRenderManager::MeshRenderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity) :
        ComponentManager(pRegistry, capacity), m_pSceneManager(nullptr), m_pResources(nullptr),
        m_pMaterialManager(nullptr), m_pAssetDatabase(nullptr), m_pLayerManager(nullptr), m_pDebug(nullptr)
    {
    }

    MeshRenderManager::~MeshRenderManager()
    {
    }

    void MeshRenderManager::OnDirtyImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        if (!pComponent.m_RenderStatic) return;
        Renderer* pRenderer = m_pSceneManager->GetRenderer();
        if (!pRenderer) return;

        MaterialData* pMaterial = m_pMaterialManager->GetMaterial(pComponent.m_Material.GetUUID());
        if (!pMaterial) return;

        const UUID pipelineID = pMaterial->GetPipelineID();

        GScene* pScene = m_pRegistry->GetUserData<GScene>();
        Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
        pRenderer->UpdateStatic(pipelineID, pComponent.m_Mesh.GetUUID(), pScene->GetEntityUUID(entity), transform.MatTransform);
    }

    void MeshRenderManager::OnDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        if (pComponent.m_RenderStatic) return;

        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
        Renderer* pRenderer = m_pSceneManager->GetRenderer();
        if (!pRenderer) return;

        Transform& transform = m_pRegistry->GetComponent<Transform>(entity);

        LayerMask mask;
        if (m_pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = m_pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_Layer.Layer(m_pLayerManager) != nullptr ? layer.m_Layer.Layer(m_pLayerManager)->m_Mask : LayerMask(0ull);
        }

        MeshData* pMeshData = m_pResources->GetResource<MeshData>(pComponent.m_Mesh.GetUUID());
        if (pMeshData == nullptr) return;

        const UUID materialID = pComponent.m_Material.GetUUID();
        if (!m_pResources->GetResource(materialID) && !m_pAssetDatabase->AssetExists(materialID))
        {
            // TODO: Set some default material
            std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
            m_pDebug->LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
            return;
        }

        if (!m_pMaterialManager->GetMaterial(materialID))
        {
            // TODO: Set some default material
            return;
        }

        GScene* pScene = m_pRegistry->GetUserData<GScene>();
        RenderData renderData;
        renderData.m_MeshID = pComponent.m_Mesh.GetUUID();
        renderData.m_MaterialID = materialID;
        renderData.m_World = transform.MatTransform;
        renderData.m_LayerMask = mask;
        renderData.m_ObjectID = pScene->GetEntityUUID(entity);
        renderData.m_SceneID = pScene->GetUUID();
        renderData.m_DepthWrite = pScene->Settings().m_DepthWrite;

        if (pScene->Settings().m_RenderLate)
            pRenderer->SubmitLate(std::move(renderData));
        else
            pRenderer->SubmitDynamic(std::move(renderData));
    }

    void MeshRenderManager::OnEnableDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        if (!pComponent.m_RenderStatic || pComponent.m_WasSubmittedForStatic) return;

        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
        Renderer* pRenderer = m_pSceneManager->GetRenderer();
        if (!pRenderer) return;

        Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
        //TransformSystem::OnUpdate(pRegistry, entity, transform);

        LayerMask mask;
        if (m_pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = m_pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_Layer.Layer(m_pLayerManager) != nullptr ? layer.m_Layer.Layer(m_pLayerManager)->m_Mask : LayerMask(0ull);
        }

        const UUID materialID = pComponent.m_Material.GetUUID();
        if (!m_pResources->GetResource(materialID) && !m_pAssetDatabase->AssetExists(materialID))
        {
            // TODO: Set some default material
            std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
            m_pDebug->LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
            return;
        }

        if (!m_pMaterialManager->GetMaterial(materialID))
        {
            // TODO: Set some default material
            return;
        }

        GScene* pScene = m_pRegistry->GetUserData<GScene>();
        RenderData renderData;
        renderData.m_MeshID = pComponent.m_Mesh.GetUUID();
        renderData.m_MaterialID = materialID;
        renderData.m_World = transform.MatTransform;
        renderData.m_LayerMask = mask;
        renderData.m_ObjectID = pScene->GetEntityUUID(entity);
        renderData.m_SceneID = pScene->GetUUID();
        renderData.m_DepthWrite = pScene->Settings().m_DepthWrite;
        pRenderer->SubmitStatic(std::move(renderData));
        pComponent.m_WasSubmittedForStatic = true;
    }

    void MeshRenderManager::OnDisableDrawImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        if (!pComponent.m_RenderStatic && !pComponent.m_WasSubmittedForStatic) return;

        Renderer* pRenderer = m_pSceneManager->GetRenderer();
        if (!pRenderer) return;

        MaterialData* pMaterial = m_pMaterialManager->GetMaterial(pComponent.m_Material.GetUUID());
        if (!pMaterial) return;

        GScene* pScene = m_pRegistry->GetUserData<GScene>();
        const UUID pipelineID = pMaterial->GetPipelineID();
        const UUID id = pScene->GetEntityUUID(entity);
        pRenderer->UnsubmitStatic(pipelineID, pComponent.m_Mesh.GetUUID(), id);
        pComponent.m_WasSubmittedForStatic = false;
    }

    void MeshRenderManager::OnValidateImpl(Utils::ECS::EntityID entity, MeshRenderer& pComponent)
    {
        const bool isActive = IsActive(entity) && m_pRegistry->EntityActiveHierarchy(entity);

        if (isActive && pComponent.m_RenderStatic && !pComponent.m_WasSubmittedForStatic)
            OnEnableDrawImpl(entity, pComponent);
        else if ((!isActive || !pComponent.m_RenderStatic) && pComponent.m_WasSubmittedForStatic)
            OnDisableDrawImpl(entity, pComponent);
    }

    void MeshRenderManager::GetReferencesImpl(std::vector<UUID>& references) const
    {
        for (size_t i = 0; i < Size(); ++i)
        {
            const MeshRenderer& meshRenderer = GetAt(i);
            const UUID material = meshRenderer.m_Material.GetUUID();
            const UUID mesh = meshRenderer.m_Mesh.GetUUID();
            if (mesh) references.push_back(mesh);
            if (material) references.push_back(material);

            Resource* pMaterial = m_pResources->GetResource(material);
            if (pMaterial) pMaterial->References(m_pSceneManager->GetEngine(), references);
        }
    }

    void MeshRenderManager::OnDeserialize(Utils::BinaryStream&)
    {
        for (size_t i = 0; i < Size(); ++i)
        {
            const MeshRenderer& mesh = GetAt(i);
            mesh.m_Material.ManualRegisterReference();
            mesh.m_Mesh.ManualRegisterReference();
        }
    }

    void MeshRenderManager::OnCopy(MeshRenderer& mesh)
    {
        mesh.m_Material.ManualRegisterReference();
        mesh.m_Mesh.ManualRegisterReference();
    }

    void MeshRenderManager::OnInitialize()
    {
        Bind(DoOnDirty, &MeshRenderManager::OnDirtyImpl);
        Bind(DoDraw, &MeshRenderManager::OnDrawImpl);
        Bind(DoOnEnableDraw, &MeshRenderManager::OnEnableDrawImpl);
        Bind(DoOnDisableDraw, &MeshRenderManager::OnDisableDrawImpl);
        Bind(DoValidate, &MeshRenderManager::OnValidateImpl);
        Bind(DoGetReferences, &MeshRenderManager::GetReferencesImpl);
    }
}
