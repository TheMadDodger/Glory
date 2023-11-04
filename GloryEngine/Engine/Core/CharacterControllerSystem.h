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
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent);
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent);
    };
}