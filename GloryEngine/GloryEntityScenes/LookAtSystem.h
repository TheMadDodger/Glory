#pragma once
#include "EntitySystemTemplate.h"
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
		virtual void OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, LookAt& pComponent) override;
		virtual std::string Name() override;
	};
}
