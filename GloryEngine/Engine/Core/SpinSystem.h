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
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Spin& pComponent);

	private:
		SpinSystem() {}
		virtual ~SpinSystem() {}
	};
}
