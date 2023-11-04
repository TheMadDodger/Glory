#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class CharacterControllerSystem
    {
    public:
        CharacterControllerSystem() = delete;

    public:
        static void OnStart(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnStop(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnValidate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
    };
}