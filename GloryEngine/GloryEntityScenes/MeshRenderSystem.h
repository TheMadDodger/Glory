#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"
#include <RenderData.h>

namespace Glory
{
    class MeshRenderSystem : public EntitySystemTemplate<MeshRenderer>
    {
    public:
        MeshRenderSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
        virtual ~MeshRenderSystem() {}

    private:
        virtual void OnDraw(Registry* pRegistry, EntityID entity, MeshRenderer& pComponent) override;
        virtual void OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, MeshRenderer& pComponent) override;
        virtual std::string Name() override;
    };
}
