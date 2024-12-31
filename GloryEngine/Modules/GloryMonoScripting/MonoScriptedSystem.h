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
    class Engine;

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

        GLORY_API static void OnBodyActivated(Engine* pEngine, UUID sceneID, UUID entityUUID);
        GLORY_API static void OnBodyDeactivated(Engine* pEngine, UUID sceneID, UUID entityUUID);

        GLORY_API static void OnContactAdded(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);
        GLORY_API static void OnContactPersisted(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);
        GLORY_API static void OnContactRemoved(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);

    private:
        MonoScriptedSystem() {}
        virtual ~MonoScriptedSystem() {}
    };
}
