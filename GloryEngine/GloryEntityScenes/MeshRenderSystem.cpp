#include "MeshRenderSystem.h"
#include "Registry.h"
#include <Game.h>
#include <Debug.h>
#include <Game.h>
#include <SerializedArrayProperty.h>
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>

namespace Glory
{
    void MeshRenderSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, MeshRenderer& pComponent)
    {
        pComponent.m_pMaterials.resize(5, nullptr);
    }

    void MeshRenderSystem::OnDraw(Registry* pRegistry, EntityID entity, MeshRenderer& pComponent)
    {
        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan

        if (!pRegistry->HasComponent<MeshFilter>(entity))
        {
            Debug::LogError("MeshRenderer: Missing MeshFilter on Entity!");
            return;
        }

        MeshFilter& meshFilter = pRegistry->GetComponent<MeshFilter>(entity);
        Transform& transform = pRegistry->GetComponent<Transform>(entity);

        LayerMask mask;
        if (pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_pLayer != nullptr ? layer.m_pLayer->m_Mask : 0;
        }

        if (meshFilter.m_pModelData == nullptr) return;
        for (size_t i = 0; i < meshFilter.m_pModelData->GetMeshCount(); i++)
        {
            if (i >= pComponent.m_pMaterials.size() || pComponent.m_pMaterials[i] == nullptr)
            {
                Debug::LogWarning("MeshRenderer: Missing Materials on MeshRenderer!");
                continue;
            }

            RenderData renderData;
            renderData.m_MeshIndex = i;
            renderData.m_pModel = meshFilter.m_pModelData;
            renderData.m_pMaterial = pComponent.m_pMaterials[i];
            renderData.m_World = transform.MatTransform;
            renderData.m_LayerMask = mask;
            Game::GetGame().GetEngine()->GetRendererModule()->Submit(renderData);
        }
    }

    void MeshRenderSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, MeshRenderer& pComponent)
    {
        properties.push_back(
            SerializedPropertyManager::GetProperty<SerializedArrayProperty<MaterialData*, MaterialData, AssetReferencePropertyTemplate<MaterialData>>>(uuid, "Materials", &pComponent.m_pMaterials, 0)
        );
    }

    std::string MeshRenderSystem::Name()
    {
        return "Mesh Renderer";
    }
}