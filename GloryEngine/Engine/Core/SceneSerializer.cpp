#include "SceneSerializer.h"
#include "SceneObject.h"
#include "SceneManager.h"
#include "GScene.h"
#include "Game.h"

namespace Glory
{
	SceneSerializer::SceneSerializer()
	{
	}

	SceneSerializer::~SceneSerializer()
	{
	}

	void SceneSerializer::Serialize(GScene* pScene, YAML::Emitter& out)
	{
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < pScene->SceneObjectsCount(); i++)
		{
			SceneObject* pObject = pScene->GetSceneObject(i);
			if (pScene->PrefabChild(pObject->GetUUID())) continue;
			Serializer::SerializeObject(pObject, out);
		}
		out << YAML::EndSeq;
	}

	Object* SceneSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string& name, Flags flags)
	{
		YAML::Node node;
		GScene* pScene = new GScene(name);
		pScene->Initialize();
		node = object["Entities"];

		for (uint32_t i = 0; i < node.size(); i++)
		{
			YAML::Node nextObject = node[i];
			Serializer::DeserializeObject(pScene, nextObject, flags);
		}
		pScene->HandleDelayedParents();

		for (size_t i = 0; i < pScene->SceneObjectsCount(); i++)
		{
			SceneObject* pObject = pScene->GetSceneObject(i);
			pObject->SetHierarchyActive();
		}

		return pScene;
	}
}
