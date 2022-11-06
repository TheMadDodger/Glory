#pragma once
#include "Components.h"

namespace GloryECS
{
	class EntityRegistry;
}

namespace Glory
{
	class SpinSystem
	{
	public:
		static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, Spin& pComponent);

	private:
		SpinSystem() {}
		virtual ~SpinSystem() {}
	};
}
