#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class TransformSystem
    {
    public:
        static void OnStart(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, Transform& pComponent);
        GLORY_API static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, Transform& pComponent);

    private:
        TransformSystem();
        virtual ~TransformSystem();

        static void CalculateMatrix(Transform& pComponent);
    };
}
