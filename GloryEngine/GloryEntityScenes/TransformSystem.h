#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"

namespace Glory
{
    class TransformSystem : public EntitySystemTemplate<Transform>
    {
    public:
        TransformSystem(Registry* pRegistry);
        virtual ~TransformSystem();

    private:
        virtual void OnUpdate(Registry* pRegistry, EntityID entity, Transform& pComponent) override;
    };
}
