#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
	class LightSystem
	{
    public:
        static void OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, LightComponent& pComponent);

    private:
        LightSystem();
        virtual ~LightSystem();
	};
}
