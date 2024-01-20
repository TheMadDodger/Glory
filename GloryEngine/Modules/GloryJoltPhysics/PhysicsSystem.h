#pragma once
#include <EntityID.h>
#include <map>
#include <functional>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class JoltPhysicsModule;
    struct PhysicsBody;

    class PhysicsSystem
    {
    private:
        PhysicsSystem() = default;
        ~PhysicsSystem();

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

        static GLORY_API PhysicsSystem* Instance();

    public:
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t)> OnBodyActivated_Callback;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t)> OnBodyDeactivated_Callback;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t, uint32_t)> OnContactAdded_Callback;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t, uint32_t)> OnContactPersisted_Callback;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t, uint32_t)> OnContactRemoved_Callback;

    private:
        static void SetupBody(JoltPhysicsModule* pPhysics, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        
    private:
        static std::map<uint32_t, std::pair<Utils::ECS::EntityRegistry*, Utils::ECS::EntityID>> m_BodyOwners;
    };
}