#pragma once
#include "EntitySystem.h"
#include "Components.h"

namespace Glory
{
    class CameraSystem : public EntitySystemTemplate<Camera>
    {
    public:
        CameraSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
        virtual ~CameraSystem() {}

    private:
        virtual void OnComponentAdded(Registry* pRegistry, EntityID entity, Camera& pComponent) override;
        virtual void OnUpdate(Registry* pRegistry, EntityID entity, Camera& pComponent) override;
        virtual void OnDraw(Registry* pRegistry, EntityID entity, Camera& pComponent) override;
    };
}
