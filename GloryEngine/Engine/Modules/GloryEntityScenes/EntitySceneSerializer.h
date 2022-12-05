#pragma once
#include <Serializer.h>
#include "EntityScene.h"

namespace Glory
{
	class EntitySceneSerializer : public SerializerTemplate<GScene>
	{
	public:
		EntitySceneSerializer();
		virtual ~EntitySceneSerializer();

	private:
		virtual void Serialize(GScene* pScene, YAML::Emitter& out) override;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object, const std::string& name) override;
	};
}
