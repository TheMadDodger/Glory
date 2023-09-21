#include "EntitySceneScenesModule.h"
#include "EntitySceneSerializer.h"
#include "EntitySceneObjectSerializer.h"
#include "Components.h"
#include "Systems.h"
#include "ScriptedComponentSerializer.h"
#include "EntityPrefabDataLoader.h"

#include <Engine.h>
#include <PhysicsModule.h>
#include <PropertyFlags.h>
#include <TypeFlags.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(EntitySceneScenesModule);

	EntitySceneScenesModule::EntitySceneScenesModule() : m_pComponentTypesInstance(nullptr), m_pPrefabLoader(nullptr)
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
		m_pComponentTypesInstance = nullptr;
		m_pPrefabLoader = nullptr;
	}

	Glory::Utils::ECS::ComponentTypes* EntitySceneScenesModule::ComponentTypesInstance() const
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
		m_pComponentTypesInstance = Glory::Utils::ECS::ComponentTypes::CreateInstance();

		/* Register component types */
		Reflect::RegisterEnum<CameraPerspective>();
		Reflect::RegisterType<MeshMaterial>();

		/* Register engine components */
		RegisterComponent<Transform>();
		RegisterComponent<LayerComponent>();
		RegisterComponent<CameraComponent>();
		RegisterComponent<MeshFilter>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<ModelRenderer>();
		RegisterComponent<LightComponent>();
		RegisterComponent<PhysicsBody>();
		RegisterComponent<CharacterController>();

		/* Always register scripted component as last to preserve execution order */
		RegisterComponent<ScriptedComponent>();

		const FieldData* pColorField = LightComponent::GetTypeData()->GetFieldData(0);
		Reflect::SetFieldFlags(pColorField, Vec4Flags::Color);

		/* Temporary components for testing */
		RegisterComponent<Spin>();
		RegisterComponent<LookAt>();

		/* Register serializers */
		Serializer::RegisterSerializer<EntitySceneSerializer>();
		Serializer::RegisterSerializer<EntitySceneObjectSerializer>();
		PropertySerializer::RegisterSerializer<ScriptedComponentSerailizer>();
		ResourceType::RegisterResource<GScene>(".gscene");

		PhysicsModule* pPhysics = m_pEngine->GetMainModule<PhysicsModule>();
		if (!pPhysics) return;

		pPhysics->RegisterActivationCallback(ActivationCallback::Activated, PhysicsSystem::OnBodyActivated);
		pPhysics->RegisterActivationCallback(ActivationCallback::Deactivated, PhysicsSystem::OnBodyDeactivated);

		pPhysics->RegisterContactCallback(ContactCallback::Added, PhysicsSystem::OnContactAdded);
		pPhysics->RegisterContactCallback(ContactCallback::Persisted, PhysicsSystem::OnContactPersisted);
		pPhysics->RegisterContactCallback(ContactCallback::Removed, PhysicsSystem::OnContactRemoved);

		m_pPrefabLoader = new EntityPrefabDataLoader();
		m_pEngine->AddOptionalModule(m_pPrefabLoader, true);
	}

	void EntitySceneScenesModule::PostInitialize()
	{
	}

	void EntitySceneScenesModule::OnCleanup()
	{
		Glory::Utils::ECS::ComponentTypes::DestroyInstance();
		m_pComponentTypesInstance = nullptr;
	}
}
