#pragma once
#include "Components.h"

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory
{
	class LookAtSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, LookAt& pComponent);

	private:
		LookAtSystem();
		virtual ~LookAtSystem();
	};
}
