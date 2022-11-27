#pragma once
#include "Components.h"

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class ScriptedSystem
    {
    public:
        static void OnAdd(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);
        static void OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent);

    private:
        ScriptedSystem() {}
        virtual ~ScriptedSystem() {}
    };
}
