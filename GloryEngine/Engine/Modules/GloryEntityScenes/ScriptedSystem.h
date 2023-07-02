#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class ScriptedSystem
    {
    public:
        static void OnAdd(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnStart(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnStop(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnValidate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);

    private:
        ScriptedSystem() {}
        virtual ~ScriptedSystem() {}
    };
}
