#pragma once
#include <Glory.h>
#include <EntityID.h>
#include <UUID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    struct MonoScriptComponent;
    struct UUIDRemapper;
    class GScene;

    class MonoScriptedSystem
    {
    public:
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnEnable(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnDisable(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnCopy(GScene* pScene, void* data, UUID componentId, UUIDRemapper& remapper);

        GLORY_API static void OnBodyActivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID);
        GLORY_API static void OnBodyDeactivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID);

        GLORY_API static void OnContactAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID);
        GLORY_API static void OnContactPersisted(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID);
        GLORY_API static void OnContactRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID);

    private:
        MonoScriptedSystem() {}
        virtual ~MonoScriptedSystem() {}
    };
}
