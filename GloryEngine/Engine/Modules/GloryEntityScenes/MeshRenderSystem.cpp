#include "MeshRenderSystem.h"
#include <Game.h>
#include <Engine.h>
#include <Debug.h>
#include <SerializedArrayProperty.h>
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>
#include <AssetManager.h>
#include <EntityRegistry.h>

namespace Glory
{
    void MeshRenderSystem::OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, MeshRenderer& pComponent)
    {
        //ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan

        Transform& transform = pRegistry->GetComponent<Transform>(entity);

        LayerMask mask;
        if (pRegistry->HasComponent<LayerComponent>(entity))
        {
            LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
            mask = layer.m_Layer.Layer() != nullptr ? layer.m_Layer.Layer()->m_Mask : 0;
        }

        MeshData* pMeshData = AssetManager::GetOrLoadAsset<MeshData>(pComponent.m_Mesh.AssetUUID());
        if (pMeshData == nullptr) return;

        if (!AssetDatabase::AssetExists(pComponent.m_Material.AssetUUID()))
        {
            // TODO: Set some default material
            std::string key = std::to_string(entity) + "_MISSING_MATERIAL";
            Debug::LogOnce(key, "MeshRenderer: Missing Materials on MeshRenderer!", Debug::LogLevel::Warning);
            return;
        }

        UUID materialUUID = pComponent.m_Material.AssetUUID();
        MaterialData* pMaterial = AssetManager::GetOrLoadAsset<MaterialData>(materialUUID);

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
        renderData.m_ObjectID = entity;
        REQUIRE_MODULE_CALL(Game::GetGame().GetEngine(), RendererModule, Submit(renderData), );
    }
}
