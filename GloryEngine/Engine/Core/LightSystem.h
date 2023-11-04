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
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, LightComponent& pComponent);

    private:
        LightSystem();
        virtual ~LightSystem();
	};
}
