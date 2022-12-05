#pragma once
#include "EntityComponentSerializer.h"
#include "Components.h"

namespace Glory
{
	class ScriptedComponentSerailizer : public EntityComponentSerializer<ScriptedComponent>
	{
	private:
		virtual void Serialize(UUID uuid, ScriptedComponent& component, YAML::Emitter& out) override;
		virtual void Deserialize(ScriptedComponent& component, YAML::Node& object) override;
	};
}
