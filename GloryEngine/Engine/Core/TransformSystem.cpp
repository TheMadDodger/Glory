#include "TransformSystem.h"
#include "GScene.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <EntityRegistry.h>

namespace Glory
{
    TransformSystem::TransformSystem()
    {
    }

    TransformSystem::~TransformSystem()
    {
    }

    void TransformSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Transform& pComponent)
    {
        CalculateMatrix(pRegistry, entity, pComponent);
    }

    void TransformSystem::OnEnable(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Transform& pComponent)
    {
        CalculateMatrix(pRegistry, entity, pComponent);
    }

    void TransformSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Transform& pComponent)
    {
        if (!pRegistry->IsEntityDirty(entity)) return;
        CalculateMatrix(pRegistry, entity, pComponent);
    }

    void Glory::TransformSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Transform& pComponent)
    {
        if (!pRegistry->IsEntityDirty(entity)) return;
        CalculateMatrix(pRegistry, entity, pComponent);
    }

    void TransformSystem::CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Transform& pComponent)
    {
        glm::mat4 startTransform = glm::identity<glm::mat4>();

        Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
        Entity parent = pRegistry->GetUserData<GScene*>()->GetEntityByEntityID(pEntityView->Parent());
        
        if (parent.IsValid() && parent.IsDirty())
        {
            const Utils::ECS::EntityID parentId = parent.GetEntityID();
            CalculateMatrix(pRegistry, parentId, parent.GetComponent<Transform>());
        }

        if (parent.IsValid())
        {
            const Transform& parentTransform = parent.GetComponent<Transform>();
            startTransform = parentTransform.MatTransform;
        }

        const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), pComponent.Scale);
        const glm::mat4 rotation = glm::inverse(glm::mat4_cast(pComponent.Rotation));
        const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), pComponent.Position);
        pComponent.MatTransform = startTransform * translation * rotation * scale;

        pRegistry->SetEntityDirty(entity, false);
    }
}
