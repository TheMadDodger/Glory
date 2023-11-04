#pragma once
#include "Components.h"
#include "RenderData.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    class ModelRenderSystem
    {
    public:
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ModelRenderer& pComponent);

    private:
        ModelRenderSystem() {}
        virtual ~ModelRenderSystem() {}
    };
}
