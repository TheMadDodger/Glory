#pragma once
#include <Serializer.h>
#include "EntityScene.h"

namespace Glory
{
	class EntitySceneSerializer : public SerializerTemplate<EntityScene>
	{
	public:
		EntitySceneSerializer();
		virtual ~EntitySceneSerializer();

	private:
		virtual void Serialize(EntityScene* pScene, YAML::Emitter& out) override;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object) override;
	};
}
