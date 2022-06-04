#include "MeshRenderSystem.h"
#include "Registry.h"
#include <Game.h>
#include <Debug.h>
#include <Game.h>
#include <SerializedArrayProperty.h>
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>
#include <AssetManager.h>

namespace Glory
{
    void MeshRenderSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, MeshRenderer& pComponent)
    {
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

        ModelData* pModelData = AssetManager::GetOrLoadAsset<ModelData>(meshFilter.m_pModelData);
        if (pModelData == nullptr) return;
        for (size_t i = 0; i < pModelData->GetMeshCount(); i++)
        {
            if (i >= pComponent.m_pMaterials.size() || !AssetDatabase::AssetExists(pComponent.m_pMaterials[i]))
            {
                // TODO: Set some default material
                std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
                Debug::LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
                continue;
            }

            UUID materialUUID = pComponent.m_pMaterials[i];
            MaterialData* pMaterial = AssetManager::GetOrLoadAsset<MaterialData>(materialUUID);

            if (pMaterial == nullptr)
            {
                // TODO: Set some default material
                return;
            }

            RenderData renderData;
            renderData.m_MeshIndex = i;
            renderData.m_pModel = pModelData;
            renderData.m_pMaterial = pMaterial;
            renderData.m_World = transform.MatTransform;
            renderData.m_LayerMask = mask;
            renderData.m_ObjectID = entity;
            Game::GetGame().GetEngine()->GetRendererModule()->Submit(renderData);
        }
    }

    void MeshRenderSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, MeshRenderer& pComponent)
    {
        properties.push_back(
            SerializedPropertyManager::GetProperty<SerializedArrayProperty<UUID, MaterialData, AssetReferencePropertyTemplate<MaterialData>>>(uuid, "Materials", &pComponent.m_pMaterials, 0)
        );
    }

    std::string MeshRenderSystem::Name()
    {
        return "Mesh Renderer";
    }
}