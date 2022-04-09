#include "EntitySceneSerializer.h"
#include "EntitySceneObject.h"
#include <Game.h>
#include "EntitySceneScenesModule.h"

namespace Glory
{
	EntitySceneSerializer::EntitySceneSerializer()
	{
	}

	EntitySceneSerializer::~EntitySceneSerializer()
	{
	}

	void EntitySceneSerializer::Serialize(GScene* pScene, YAML::Emitter& out)
	{
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pScene->SceneObjectsCount(); i++)
		{
			EntitySceneObject* pObject = (EntitySceneObject*)pScene->GetSceneObject(i);
			Serializer::SerializeObject(pObject, out);
		}
		out << YAML::EndSeq;
	}

	Object* EntitySceneSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string& name)
	{
		YAML::Node node;
		EntityScene* pScene = new EntityScene(name);
		pScene->Initialize();
		node = object["Entities"];

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node nextObject = node[i];
			Serializer::DeserializeObject(pScene, nextObject);
		}

		return pScene;
	}
}
