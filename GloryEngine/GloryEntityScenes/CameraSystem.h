#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"

namespace Glory
{
    class CameraSystem : public EntitySystemTemplate<CameraComponent>
    {
    public:
        CameraSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
        virtual ~CameraSystem() {}

    private:
        virtual void OnComponentAdded(Registry* pRegistry, EntityID entity, CameraComponent& pComponent) override;
        virtual void OnComponentRemoved(Registry* pRegistry, EntityID entity, CameraComponent& pComponent) override;
        virtual void OnUpdate(Registry* pRegistry, EntityID entity, CameraComponent& pComponent) override;
        virtual void OnDraw(Registry* pRegistry, EntityID entity, CameraComponent& pComponent) override;
        virtual void OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, CameraComponent& pComponent) override;
        virtual std::string Name();

        size_t CalcHash(CameraComponent& pComponent);
    };
}
