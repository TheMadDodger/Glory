#pragma once
#include "Components.h"

namespace GloryECS
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
        static void OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
        static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent);
    };
}