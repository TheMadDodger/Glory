#include "EntitySceneScenesModule.h"
#include "EntitySceneSerializer.h"
#include "EntitySceneObjectSerializer.h"
#include "Components.h"
#include "ScriptedComponentSerializer.h"
#include <PropertyFlags.h>
#include <TypeFlags.h>
#include "PhysicsSystem.h"

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(EntitySceneScenesModule, 0, 1);

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

		/* Register component types */
		GloryReflect::Reflect::RegisterEnum<CameraPerspective>();
		GloryReflect::Reflect::RegisterType<MeshMaterial>();

		/* Register engine components */
		RegisterComponent<Transform>();
		RegisterComponent<LayerComponent>();
		RegisterComponent<CameraComponent>();
		RegisterComponent<MeshFilter>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<ModelRenderer>();
		RegisterComponent<LightComponent>();
		RegisterComponent<ScriptedComponent>();
		RegisterComponent<PhysicsBody>();

		const GloryReflect::FieldData* pColorField = LightComponent::GetTypeData()->GetFieldData(0);
		GloryReflect::Reflect::SetFieldFlags(pColorField, Vec4Flags::Color);

		/* Temporary components for testing */
		RegisterComponent<Spin>();
		RegisterComponent<LookAt>();

		/* Register serializers */
		Serializer::RegisterSerializer<EntitySceneSerializer>();
		Serializer::RegisterSerializer<EntitySceneObjectSerializer>();
		Serializer::RegisterSerializer<ScriptedComponentSerailizer>();
		ResourceType::RegisterResource<GScene>(".gscene");

		m_pEngine->GetPhysicsModule()->RegisterActivationCallback(ActivationCallback::Activated, PhysicsSystem::OnBodyActivated);
		m_pEngine->GetPhysicsModule()->RegisterActivationCallback(ActivationCallback::Deactivated, PhysicsSystem::OnBodyDeactivated);
	}

	void EntitySceneScenesModule::PostInitialize()
	{
	}

	void EntitySceneScenesModule::OnCleanup()
	{
		GloryECS::ComponentTypes::DestroyInstance();
		m_pComponentTypesInstance = nullptr;
	}
}
