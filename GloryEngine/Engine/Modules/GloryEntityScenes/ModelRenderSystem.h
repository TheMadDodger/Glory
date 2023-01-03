#pragma once
#include "Components.h"
#include <RenderData.h>

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class ModelRenderSystem
    {
    public:
        static void OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, ModelRenderer& pComponent);

    private:
        ModelRenderSystem() {}
        virtual ~ModelRenderSystem() {}
    };
}
