#pragma once
#include "EntitySystem.h"
#include "Components.h"

namespace Glory
{
	class SpinSystem : public EntitySystemTemplate<Spin>
	{
	public:
		SpinSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry) {}
		virtual ~SpinSystem() {}

	private:
		virtual void OnDraw(Registry* pRegistry, EntityID entity, Spin& pComponent) override;
	};
}
