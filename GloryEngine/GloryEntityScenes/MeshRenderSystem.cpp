#include "MeshRenderSystem.h"
#include <Game.h>
#include "Registry.h"
#include <Debug.h>
#include <Game.h>

namespace Glory
{
    void MeshRenderSystem::OnDraw(Registry* pRegistry, EntityID entity, MeshRenderer& pComponent)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObjectTest ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        Window* pWindow = Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow();

        int width, height;
        pWindow->GetDrawableSize(&width, &height);

        ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        //ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan

        if (!pRegistry->HasComponent<MeshFilter>(entity))
        {
            Debug::LogError("MeshRenderer: Missing MeshFilter on Entity!");
            return;
        }

        MeshFilter& meshFilter = pRegistry->GetComponent<MeshFilter>(entity);

        for (size_t i = 0; i < meshFilter.m_pModelData->GetMeshCount(); i++)
        {
            if (i >= pComponent.m_pMaterials.size())
            {
                Debug::LogWarning("MeshRenderer: Missing Materials on MeshRenderer!");
                continue;
            }

            RenderData renderData;
            renderData.m_MeshIndex = i;
            renderData.m_pModel = meshFilter.m_pModelData;
            renderData.m_pMaterial = pComponent.m_pMaterials[i];
            renderData.m_UBO = ubo;
            Game::GetGame().GetEngine()->GetRendererModule()->Submit(renderData);
        }
    }
}