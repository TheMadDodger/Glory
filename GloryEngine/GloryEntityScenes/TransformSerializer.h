#pragma once
#include "EntityComponentSerializer.h"
#include "Components.h"

namespace Glory
{
	class TransformSerializer : public EntityComponentSerializer<Transform>
	{
	public:
		TransformSerializer();
		virtual ~TransformSerializer();

	private:
		virtual void Serialize(Transform& component, YAML::Emitter& out) override;
		virtual void Deserialize(Transform& component, YAML::Node& object) override;
	};
}
