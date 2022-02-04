#pragma once
#include "EntitySystemTemplate.h"
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
		virtual void OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, Spin& pComponent) override;
		virtual std::string Name() override;
	};
}
