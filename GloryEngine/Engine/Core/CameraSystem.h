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
        static void OnComponentAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnComponentRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent);
        static std::string Name();

        static uint32_t CalcHash(CameraComponent& pComponent);
    };
}
