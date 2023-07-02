#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory
{
	class SpinSystem
	{
	public:
		static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, Spin& pComponent);

	private:
		SpinSystem() {}
		virtual ~SpinSystem() {}
	};
}
