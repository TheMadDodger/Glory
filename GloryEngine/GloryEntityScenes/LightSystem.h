#pragma once
#include "Components.h"

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
	class LightSystem
	{
    public:
        static void OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, LightComponent& pComponent);

    private:
        LightSystem();
        virtual ~LightSystem();
	};
}
