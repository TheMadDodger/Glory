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
        static void OnAdd(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent);

    private:
        ScriptedSystem() {}
        virtual ~ScriptedSystem() {}
    };
}
