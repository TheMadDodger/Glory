#pragma once
#include "Components.h"

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class PhysicsSystem
    {
    public:
        PhysicsSystem() = delete;

    public:
        static void OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);

    private:
        static void SetupBody(PhysicsModule* pPhysics, GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
    };
}