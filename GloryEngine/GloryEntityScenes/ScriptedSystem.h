#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"

namespace Glory
{
    class ScriptedSystem : public EntitySystemTemplate<ScriptedComponent>
    {
    public:
        ScriptedSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
        virtual ~ScriptedSystem() {}

    private:
        virtual void OnComponentAdded(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent) override;
        virtual void OnComponentRemoved(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent) override;
        virtual void OnUpdate(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent) override;
        virtual void OnDraw(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent) override;
        virtual void OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, ScriptedComponent& pComponent) override;
        virtual std::string Name();
    };
}
