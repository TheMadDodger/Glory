#pragma once
#include "Components.h"

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class TransformSystem
    {
    public:
        static void OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, Transform& pComponent);
        static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, Transform& pComponent);

    private:
        TransformSystem();
        virtual ~TransformSystem();

        static void CalculateMatrix(Transform& pComponent);
    };
}
