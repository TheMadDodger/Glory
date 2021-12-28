#include "EntitySceneObjectSerializer.h"
#include "EntityComponentObject.h"
#include "EntitySceneScenesModule.h"

namespace Glory
{
	EntitySceneObjectSerializer::EntitySceneObjectSerializer()
	{
	}

	EntitySceneObjectSerializer::~EntitySceneObjectSerializer()
	{
	}

	void EntitySceneObjectSerializer::Serialize(EntitySceneObject* pObject, YAML::Emitter& out)
	{
		out << YAML::Key << "Name";
		out << YAML::Value << pObject->Name();
		out << YAML::Key << "Components";
		out << YAML::Value << YAML::BeginSeq;
		pObject->GetEntityHandle().ForEachComponent([&](Registry* pRegistry, EntityID entityID, EntityComponentData* pComponentData)
		{
			EntityComponentObject* pComponentObject = new EntityComponentObject(pComponentData);
			Serializer::SerializeObject(pComponentObject, out);
			delete pComponentObject;
		});
		out << YAML::EndSeq;
	}

	Object* EntitySceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object)
	{
		YAML::Node node;
		std::string name;
		UUID uuid;
		YAML_READ(object, node, Name, name, std::string);
		YAML_READ(object, node, UUID, uuid, uint64_t);

		EntityScene* pScene = (EntityScene*)pParent;
		EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject(name, uuid);
		node = object["Components"];

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node nextObject = node[i];
			Serializer::DeserializeObject(pObject, nextObject);
		}

		return pScene;
	}
}
