#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
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
        static void OnStart(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnStop(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnValidate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);

        static void OnBodyActivated(uint32_t bodyID);
        static void OnBodyDeactivated(uint32_t bodyID);

        static void OnContactAdded(uint32_t body1ID, uint32_t body2ID);
        static void OnContactPersisted(uint32_t body1ID, uint32_t body2ID);
        static void OnContactRemoved(uint32_t body1ID, uint32_t body2ID);

        static void AddBody(uint32_t bodyID, Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity);
        static void RemoveBody(uint32_t bodyID);

    private:
        static void SetupBody(PhysicsModule* pPhysics, Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent);
        
    private:
        static std::map<uint32_t, std::pair<Glory::Utils::ECS::EntityRegistry*, EntityID>> m_BodyOwners;
    };
}