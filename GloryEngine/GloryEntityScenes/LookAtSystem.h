#pragma once
#include "EntitySystem.h"
#include "Components.h"

namespace Glory
{
	class LookAtSystem : public EntitySystemTemplate<LookAt>
	{
	public:
		LookAtSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
		virtual ~LookAtSystem() {}

	private:
		virtual void OnUpdate(Registry* pRegistry, EntityID entity, LookAt& pComponent) override;
	};
}
