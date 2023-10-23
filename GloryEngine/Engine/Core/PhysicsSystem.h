#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class PhysicsModule;

    class PhysicsSystem
    {
    public:
        PhysicsSystem() = delete;

    public:
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);

        static void OnBodyActivated(uint32_t bodyID);
        static void OnBodyDeactivated(uint32_t bodyID);

        static void OnContactAdded(uint32_t body1ID, uint32_t body2ID);
        static void OnContactPersisted(uint32_t body1ID, uint32_t body2ID);
        static void OnContactRemoved(uint32_t body1ID, uint32_t body2ID);

        static void AddBody(uint32_t bodyID, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity);
        static void RemoveBody(uint32_t bodyID);

    private:
        static void SetupBody(PhysicsModule* pPhysics, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        
    private:
        static std::map<uint32_t, std::pair<Utils::ECS::EntityRegistry*, Utils::ECS::EntityID>> m_BodyOwners;
    };
}