#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class CameraSystem
    {
    private:
        CameraSystem();
        virtual ~CameraSystem();

    public:
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnComponentAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnComponentRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnEnableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnDisableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void Focus(Transform& transform, CameraComponent& pComponent, const BoundingSphere& boundingSphere);

        static std::string Name();
    };
}
