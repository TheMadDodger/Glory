#pragma once
#include "Components.h"
#include <RenderData.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class MeshRenderSystem
    {
    public:
        static void OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, MeshRenderer& pComponent);

    private:
        MeshRenderSystem() {}
        virtual ~MeshRenderSystem() {}
    };
}
