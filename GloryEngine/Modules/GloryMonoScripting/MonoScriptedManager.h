#pragma once
#include "mono_visibility.h"

#include "MonoComponents.h"

#include <EntityRegistry.h>
#include <ComponentManager.h>

namespace Glory
{
    namespace Utils
    {
        class BinaryStream;
    }

    struct UUIDRemapper;
    class GScene;
    class IEngine;

    class CoreLibManager;
    class MonoScriptManager;

    class MonoScriptedManager : public Utils::ECS::ComponentManager<MonoScriptComponent>
    {
    public:
        MonoScriptedManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
        virtual ~MonoScriptedManager();

    public:
        virtual void OnSerialize(Utils::BinaryStream& stream) const override;
        virtual void OnDeserialize(Utils::BinaryStream& stream) override;
        virtual void OnCopy(MonoScriptComponent& script) override;

        void UnpackDataInto(const void* data, MonoScriptComponent& newComponent) override;
        virtual void SerializeDense(Utils::BinaryStream& stream) const override;
        virtual void DeserializeDense(Utils::BinaryStream& stream) override;

        void OnStartImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        void OnStopImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        void OnValidateImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        void OnEnableImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        void OnDisableImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        void OnUpdateImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent, float);
        void OnDrawImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent);
        static void OnCopy(GScene* pScene, void* data, UUID componentId, UUIDRemapper& remapper);

        void GetReferencesImpl(std::vector<UUID>& references) const;

        GLORY_MONO_API static void OnBodyActivated(IEngine* pEngine, UUID sceneID, UUID entityUUID);
        GLORY_MONO_API static void OnBodyDeactivated(IEngine* pEngine, UUID sceneID, UUID entityUUID);

        GLORY_MONO_API static void OnContactAdded(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);
        GLORY_MONO_API static void OnContactPersisted(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);
        GLORY_MONO_API static void OnContactRemoved(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID);

    private:
        void OnInitialize() override;

    private:
        friend class GloryMonoScipting;
        CoreLibManager* m_pCoreLibManager = nullptr;
        MonoScriptManager* m_pScriptManager = nullptr;
    };
}
