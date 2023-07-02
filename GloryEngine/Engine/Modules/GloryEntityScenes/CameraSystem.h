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
        static void OnComponentAdded(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnComponentRemoved(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static std::string Name();

        static uint32_t CalcHash(CameraComponent& pComponent);
    };
}
