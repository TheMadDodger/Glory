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
		static void OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, LookAt& pComponent);

	private:
		LookAtSystem();
		virtual ~LookAtSystem();
	};
}
