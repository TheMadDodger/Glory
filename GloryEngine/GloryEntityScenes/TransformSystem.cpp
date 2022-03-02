#include "TransformSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <SerializedPropertyManager.h>

namespace Glory
{
    TransformSystem::TransformSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry)
    {
        m_Hidden = true;
    }

    TransformSystem::~TransformSystem()
    {
    }

    void Glory::TransformSystem::OnUpdate(Registry* pRegistry, EntityID entity, Transform& pComponent)
    {
        glm::mat4 startTransform = glm::identity<glm::mat4>();
        if (pComponent.Parent != nullptr)
        {
            startTransform = pComponent.Parent->MatTransform;
        }

        glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), pComponent.Scale);
        glm::mat4 rotation = glm::mat4_cast(pComponent.Rotation);
        //glm::mat4 rotation = glm::eulerAngleXYX(90.0f, 90.0f, 0.0f);
        glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), pComponent.Position);
        pComponent.MatTransform = startTransform * translation * rotation * scale;
    }

    void TransformSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, Transform& pComponent)
    {
        properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec3>>(uuid, std::string("Position"), &pComponent.Position, 0));
        properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::quat>>(uuid, std::string("Rotation"), &pComponent.Rotation, 0));
        properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec3>>(uuid, std::string("Scale"), &pComponent.Scale, 0));
    }

    std::string TransformSystem::Name()
    {
        return "Transform";
    }
}
