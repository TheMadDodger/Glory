#pragma once
#include "Components.h"
#include <RenderData.h>

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class MeshRenderSystem
    {
    public:
        static void OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, MeshRenderer& pComponent);

    private:
        MeshRenderSystem() {}
        virtual ~MeshRenderSystem() {}
    };
}
