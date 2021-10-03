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
        pComponent.MatTransform = glm::scale(glm::identity<glm::mat4>(), pComponent.Scale);
        //pComponent.MatTransform = glm::rotate(pComponent.MatTransform, pComponent.Rotation);
        pComponent.MatTransform = glm::translate(pComponent.MatTransform, pComponent.Position);
    }
}
