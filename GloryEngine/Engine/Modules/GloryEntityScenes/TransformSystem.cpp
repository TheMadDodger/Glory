#include "TransformSystem.h"
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

    void Glory::TransformSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, Transform& pComponent)
    {
        glm::mat4 startTransform = glm::identity<glm::mat4>();
        if (pComponent.Parent.IsValid())
        {
            Transform& parentTransform = pComponent.Parent.GetComponent<Transform>();
            startTransform = parentTransform.MatTransform;
        }

        glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), pComponent.Scale);
        glm::mat4 rotation = glm::inverse(glm::mat4_cast(pComponent.Rotation));
        glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), pComponent.Position);
        pComponent.MatTransform = startTransform * translation * rotation * scale;
    }
}
