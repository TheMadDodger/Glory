#include "EntitySceneScenesModule.h"
#include "EntitySceneSerializer.h"
#include "EntitySceneObjectSerializer.h"
#include "Components.h"
#include <TypeFlags.h>

namespace Glory
{
	EntitySceneScenesModule::EntitySceneScenesModule()
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
	}

	GScene* EntitySceneScenesModule::CreateScene(const std::string& sceneName)
	{
		return new EntityScene(sceneName);
	}

	GScene* EntitySceneScenesModule::CreateScene(const std::string& sceneName, UUID uuid)
	{
		return new EntityScene(sceneName, uuid);
	}

	EntitySceneObject* EntitySceneScenesModule::CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return (EntitySceneObject*)CreateObject(pScene, name, uuid);
	}

	SceneObject* EntitySceneScenesModule::GetSceneObjectFromObjectID(uint32_t objectID)
	{
		for (size_t i = 0; i < m_pOpenScenes.size(); i++)
		{
			EntityScene* pEntityScene = (EntityScene*)m_pOpenScenes[i];
			EntitySceneObject* pEntityObject = pEntityScene->GetEntitySceneObjectFromEntityID(objectID);
			if (pEntityObject) return pEntityObject;
		}

		return nullptr;
	}

	void EntitySceneScenesModule::Initialize()
	{
		GloryReflect::Reflect::RegisterType<MeshMaterial>();
		GloryReflect::Reflect::RegisterType<RecursionTest3>();
		GloryReflect::Reflect::RegisterType<RecursionTest2>();
		GloryReflect::Reflect::RegisterType<RecursionTest>();

		GloryReflect::Reflect::RegisterType<Transform>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<MeshFilter>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<MeshRenderer>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<CameraComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<Spin>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LayerComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LightComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LookAt>(TypeFlag::TF_Component);

		Serializer::RegisterSerializer<EntitySceneSerializer>();
		Serializer::RegisterSerializer<EntitySceneObjectSerializer>();
		ResourceType::RegisterResource<GScene>(".gscene");
	}

	void EntitySceneScenesModule::PostInitialize()
	{
		size_t hash = ResourceType::GetHash<ImageData>();
		int a = 0;
	}

	void EntitySceneScenesModule::OnCleanup()
	{
	}
}
