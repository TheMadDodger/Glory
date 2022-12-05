#pragma once
#include "Components.h"

namespace GloryECS
{
	class EntityRegistry;
}

namespace Glory
{
	class LookAtSystem
	{
	public:
		static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, LookAt& pComponent);

	private:
		LookAtSystem();
		virtual ~LookAtSystem();
	};
}
