#pragma once
#include "Components.h"
#include <RenderData.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class ModelRenderSystem
    {
    public:
        static void OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ModelRenderer& pComponent);

    private:
        ModelRenderSystem() {}
        virtual ~ModelRenderSystem() {}
    };
}
