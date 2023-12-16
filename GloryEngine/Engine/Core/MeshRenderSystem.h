#pragma once
#include "Components.h"
#include "RenderData.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class MeshRenderSystem
    {
    public:
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MeshRenderer& pComponent);
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MeshRenderer& pComponent);

    private:
        MeshRenderSystem() {}
        virtual ~MeshRenderSystem() {}
    };
}
