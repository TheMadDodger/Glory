#pragma once
#include <EntityID.h>
#include <UUID.h>
#include <map>
#include <functional>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class Engine;
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

        static void OnBodyActivated(JoltPhysicsModule* pPhysics, uint32_t bodyID);
        static void OnBodyDeactivated(JoltPhysicsModule* pPhysics, uint32_t bodyID);

        static void OnContactAdded(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID);
        static void OnContactPersisted(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID);
        static void OnContactRemoved(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID);

        static void AddToSceneIDsCache(UUID entityUUID, UUID sceneID);
        static void RemoveFromSceneIDsCache(UUID entityUUID);

        static GLORY_API PhysicsSystem* Instance();

    public:
        std::function<void(Engine*, UUID, UUID)> OnBodyActivated_Callback;
        std::function<void(Engine*, UUID, UUID)> OnBodyDeactivated_Callback;
        std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnContactAdded_Callback;
        std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnContactPersisted_Callback;
        std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnContactRemoved_Callback;

    private:
        static void SetupBody(JoltPhysicsModule* pPhysics, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent);
        
    private:
        static std::map<UUID, UUID> m_CachedSceneIDs;
    };
}