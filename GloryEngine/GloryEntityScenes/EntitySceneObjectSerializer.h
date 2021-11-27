#pragma once
#include <Serializer.h>
#include "EntitySceneObject.h"

namespace Glory
{
	class EntitySceneObjectSerializer : public SerializerTemplate<EntitySceneObject>
	{
	public:
		EntitySceneObjectSerializer();
		virtual ~EntitySceneObjectSerializer();

	private:
		virtual void Serialize(EntitySceneObject* pObject, YAML::Emitter& out) override;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object) override;
	};
}
