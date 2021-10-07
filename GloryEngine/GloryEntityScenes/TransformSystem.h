#pragma once
#include "EntitySystem.h"
#include "Components.h"
#include <Game.h>

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

	class TriangleSystem : public EntitySystemTemplate<Triangle>
	{
	public:
        TriangleSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
		virtual ~TriangleSystem() {}

    private:
        virtual void OnDraw(Registry* pRegistry, EntityID entity, Triangle& pComponent) override
        {
            RenderData renderData;
            renderData.m_MeshIndex = 0;
            renderData.m_pModel = pComponent.m_pModelData;
            Game::GetGame().GetEngine()->GetRendererModule()->Submit(renderData);
        }
	};
}
