#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"

namespace Glory
{
	class LightSystem : public EntitySystemTemplate<LightComponent>
	{
    public:
        LightSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
        virtual ~LightSystem() {}

    private:
        virtual void OnComponentAdded(Registry* pRegistry, EntityID entity, LightComponent& pComponent) override;
        virtual void OnComponentRemoved(Registry* pRegistry, EntityID entity, LightComponent& pComponent) override;
        virtual void OnUpdate(Registry* pRegistry, EntityID entity, LightComponent& pComponent) override;
        virtual void OnDraw(Registry* pRegistry, EntityID entity, LightComponent& pComponent) override;
        virtual void OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, LightComponent& pComponent) override;
        virtual std::string Name() override;
	};
}
