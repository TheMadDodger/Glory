#include "EntitySceneScenesModule.h"
#include "EntitySceneSerializer.h"
#include "EntitySceneObjectSerializer.h"
#include "Components.h"
#include "ScriptedComponentSerializer.h"
#include <TypeFlags.h>

namespace Glory
{
	EntitySceneScenesModule::EntitySceneScenesModule() : m_pComponentTypesInstance(nullptr)
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
	}

	GloryECS::ComponentTypes* EntitySceneScenesModule::ComponentTypesInstance() const
	{
		return m_pComponentTypesInstance;
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

	SceneObject* EntitySceneScenesModule::GetSceneObjectFromObjectID(uint64_t objectID)
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
		m_pComponentTypesInstance = GloryECS::ComponentTypes::CreateInstance();

		// Register engine components
		GloryECS::ComponentTypes::RegisterComponent<Transform>();
		GloryECS::ComponentTypes::RegisterComponent<MeshRenderer>();
		GloryECS::ComponentTypes::RegisterComponent<MeshFilter>();
		GloryECS::ComponentTypes::RegisterComponent<CameraComponent>();
		GloryECS::ComponentTypes::RegisterComponent<LookAt>();
		GloryECS::ComponentTypes::RegisterComponent<Spin>();
		GloryECS::ComponentTypes::RegisterComponent<LightComponent>();
		GloryECS::ComponentTypes::RegisterComponent<ScriptedComponent>();

		// Register component types
		GloryReflect::Reflect::RegisterEnum<CameraPerspective>();
		GloryReflect::Reflect::RegisterType<MeshMaterial>();

		GloryReflect::Reflect::RegisterType<Transform>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<MeshFilter>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<MeshRenderer>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<CameraComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<Spin>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LayerComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LightComponent>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<LookAt>(TypeFlag::TF_Component);
		GloryReflect::Reflect::RegisterType<ScriptedComponent>(TypeFlag::TF_Component);

		// Register serializers
		Serializer::RegisterSerializer<EntitySceneSerializer>();
		Serializer::RegisterSerializer<EntitySceneObjectSerializer>();
		Serializer::RegisterSerializer<ScriptedComponentSerailizer>();
		ResourceType::RegisterResource<GScene>(".gscene");
	}

	void EntitySceneScenesModule::PostInitialize()
	{
		size_t hash = ResourceType::GetHash<ImageData>();
		int a = 0;
	}

	void EntitySceneScenesModule::OnCleanup()
	{
		GloryECS::ComponentTypes::DestroyInstance();
		m_pComponentTypesInstance = nullptr;
	}
}
