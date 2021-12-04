#include "TransformSystem.h"
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
    TransformSystem::TransformSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry)
    {
    }

    TransformSystem::~TransformSystem()
    {
    }

    void Glory::TransformSystem::OnUpdate(Registry* pRegistry, EntityID entity, Transform& pComponent)
    {
        glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), pComponent.Scale);
        glm::mat4 rotation = glm::toMat4(pComponent.Rotation);
        glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), pComponent.Position);
        pComponent.MatTransform = translation * rotation * scale;
    }
}
