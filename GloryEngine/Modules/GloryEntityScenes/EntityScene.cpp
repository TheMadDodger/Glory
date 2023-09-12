#include <glm/glm.hpp>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include "EntitySceneObject.h"
#include "EntityPrefabData.h"
#include "PropertySerializer.h"
#include "UUIDRemapper.h"
#include "DistributedRandom.h"

#include <EntityView.h>
#include <TypeData.h>
#include <glm/glm.hpp>
#include <PropertySerializer.h>
#include <NodeRef.h>

namespace Glory
{
	EntityScene::EntityScene() : m_Valid(true), GScene("New Scene"), m_Registry(this) //: m_pJobPool(nullptr)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName) : m_Valid(true), GScene(sceneName), m_Registry(this)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName, UUID uuid) : m_Valid(true), GScene(sceneName, uuid), m_Registry(this)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::~EntityScene()
	{
		m_Valid = false;
	}

	Entity EntityScene::CreateEntity(UUID uuid, UUID transUUID)
	{
		EntityID entityID = m_Registry.CreateEntity<Transform>(transUUID);
		return Entity(entityID, this);
	}

	EntitySceneObject* EntityScene::GetEntitySceneObjectFromEntityID(EntityID entity)
	{
		if (m_EntityIDToObject.find(entity) == m_EntityIDToObject.end()) return nullptr;
		return m_EntityIDToObject[entity];
	}

	EntityRegistry* EntityScene::GetRegistry()
	{
		return &m_Registry;
	}

	bool EntityScene::IsValid() const
	{
		return m_Valid;
	}

	SceneObject* EntityScene::InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab)
	{
		EntityPrefabData* pEntityPrefab = (EntityPrefabData*)pPrefab;

		UUID uuid = UUID();
		const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
		const uint32_t second32Bits = uint32_t(uuid >> 32);
		const uint32_t seed = first32Bits & second32Bits;
		UUIDRemapper remapper{ seed };
		remapper.EnforceRemap(pEntityPrefab->RootNode().OriginalUUID(), uuid);
		return InstantiatePrefab(pParent, (EntityPrefabData*)pPrefab, RandomDevice::Seed());
	}

	SceneObject* EntityScene::InstantiatePrefab(SceneObject* pParent, EntityPrefabData* pPrefab, uint32_t remapSeed)
	{
		UUIDRemapper remapper{ remapSeed };
		const PrefabNode& rootNode = pPrefab->RootNode();
		EntitySceneObject* pInstantiatedPrefab = InstantiatePrefabNode((EntitySceneObject*)pParent, rootNode, remapper);
		SetPrefab(pInstantiatedPrefab, pPrefab->GetUUID());
		return pInstantiatedPrefab;
	}

	SceneObject* EntityScene::InstantiatePrefab(SceneObject* pParent, EntityPrefabData* pPrefab, UUIDRemapper& remapper)
	{
		const PrefabNode& rootNode = pPrefab->RootNode();
		EntitySceneObject* pInstantiatedPrefab = InstantiatePrefabNode((EntitySceneObject*)pParent, rootNode, remapper);
		SetPrefab(pInstantiatedPrefab, pPrefab->GetUUID());
		return pInstantiatedPrefab;
	}

	void EntityScene::Initialize()
	{
		// Register Invocations
		// Transform
		m_Registry.RegisterInvokaction<Transform>(Glory::Utils::ECS::InvocationType::Start, TransformSystem::OnStart);
		m_Registry.RegisterInvokaction<Transform>(Glory::Utils::ECS::InvocationType::Update, TransformSystem::OnUpdate);
		
		// Camera
		m_Registry.RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::OnAdd, CameraSystem::OnComponentAdded);
		m_Registry.RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::OnRemove, CameraSystem::OnComponentRemoved);
		m_Registry.RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::Update, CameraSystem::OnUpdate);
		m_Registry.RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::Draw, CameraSystem::OnDraw);

		// Light
		m_Registry.RegisterInvokaction<LightComponent>(Glory::Utils::ECS::InvocationType::Draw, LightSystem::OnDraw);

		// LookAt
		m_Registry.RegisterInvokaction<LookAt>(Glory::Utils::ECS::InvocationType::Update, LookAtSystem::OnUpdate);

		// MeshRenderer
		m_Registry.RegisterInvokaction<MeshRenderer>(Glory::Utils::ECS::InvocationType::Draw, MeshRenderSystem::OnDraw);

		// Spin
		m_Registry.RegisterInvokaction<Spin>(Glory::Utils::ECS::InvocationType::Update, SpinSystem::OnUpdate);

		/* Physics Bodies */
		m_Registry.RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Start, PhysicsSystem::OnStart);
		m_Registry.RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Stop, PhysicsSystem::OnStop);
		m_Registry.RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::OnRemove, PhysicsSystem::OnStop);
		m_Registry.RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::OnValidate, PhysicsSystem::OnValidate);
		m_Registry.RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Update, PhysicsSystem::OnUpdate);

		/* Character controllers */
		m_Registry.RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Start, CharacterControllerSystem::OnStart);
		m_Registry.RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Stop, CharacterControllerSystem::OnStop);
		m_Registry.RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::OnRemove, CharacterControllerSystem::OnStop);
		m_Registry.RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::OnValidate, CharacterControllerSystem::OnValidate);
		m_Registry.RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Update, CharacterControllerSystem::OnUpdate);

		// Scripted
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::OnAdd, ScriptedSystem::OnAdd);
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::Update, ScriptedSystem::OnUpdate);
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::Draw, ScriptedSystem::OnDraw);
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::Start, ScriptedSystem::OnStart);
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::Stop, ScriptedSystem::OnStop);
		m_Registry.RegisterInvokaction<ScriptedComponent>(Glory::Utils::ECS::InvocationType::OnValidate, ScriptedSystem::OnValidate);
	}

	void EntityScene::OnTick()
	{
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Update);
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntityScene::OnPaint()
	{
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Draw);
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	SceneObject* EntityScene::CreateObject(const std::string& name)
	{
		UUID uuid = UUID();
		Entity entity = CreateEntity(uuid, UUID());
		return new EntitySceneObject(entity, name, uuid);
	}

	SceneObject* EntityScene::CreateObject(const std::string& name, UUID uuid, UUID uuid2)
	{
		Entity entity = CreateEntity(uuid, uuid2);
		return new EntitySceneObject(entity, name, uuid);
	}

	void EntityScene::OnDeleteObject(SceneObject* pObject)
	{
	}

	void EntityScene::OnObjectAdded(SceneObject* pObject)
	{
		EntitySceneObject* pEntityObject = (EntitySceneObject*)pObject;
		EntityID entity = pEntityObject->GetEntityHandle().GetEntityID();
		m_EntityIDToObject[entity] = pEntityObject;
	}

	void EntityScene::Start()
	{
		m_Registry.InvokeAll(InvocationType::Start);
	}

	void EntityScene::Stop()
	{
		m_Registry.InvokeAll(InvocationType::Stop);
	}

	EntitySceneObject* EntityScene::InstantiatePrefabNode(EntitySceneObject* pParent, const PrefabNode& node, UUIDRemapper& remapper)
	{
		const UUID objectID = remapper(node.OriginalUUID());
		const UUID transformID = remapper(node.TransformUUID());
		EntitySceneObject* pObject = (EntitySceneObject*)CreateEmptyObject(node.Name(), objectID, transformID);
		if (pParent)
			pObject->SetParent(pParent);

		pObject->SetActive(node.ActiveSelf());

		const std::string& serializedComponents = node.SerializedComponents();
		YAML::Node components = YAML::Load(serializedComponents);

		const uint32_t transformTypeHash = ResourceType::GetHash(typeid(Transform));
		const uint32_t scriptedTypeHash = ResourceType::GetHash(typeid(ScriptedComponent));

		size_t currentComponentIndex = 0;
		for (size_t i = 0; i < components.size(); ++i)
		{
			YAML::Node nextObject = components[i];
			YAML::Node subNode;
			uint32_t typeHash = 0;
			UUID originalUUID = 0;
			std::string typeName = "";
			YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);
			YAML_READ(nextObject, subNode, UUID, originalUUID, uint64_t);

			Entity entityHandle = pObject->GetEntityHandle();
			EntityID entity = entityHandle.GetEntityID();
			EntityRegistry* pRegistry = GetRegistry();

			UUID compUUID = remapper(originalUUID);

			void* pComponentAddress = nullptr;
			if (typeHash != transformTypeHash) pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
			else
			{
				Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
				compUUID = pEntityView->ComponentUUIDAt(0);
				pComponentAddress = pRegistry->GetComponentAddress(entity, compUUID);
			}

			const Utils::Reflect::TypeData* pTypeData = Utils::Reflect::Reflect::GetTyeData(typeHash);
			YAML::Node originalProperties = nextObject["Properties"];
			if (typeHash != scriptedTypeHash)
			{
				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, originalProperties);
			}
			else
			{
				YAML::Node finalProperties = YAML::Node(YAML::NodeType::Map);

				Utils::NodeRef originalPropertiesRef = originalProperties;
				Utils::NodeRef finalPropertiesRef = finalProperties;

				Utils::NodeValueRef props = originalPropertiesRef.ValueRef();
				Utils::NodeValueRef finalProps = finalPropertiesRef.ValueRef();

				finalPropertiesRef["m_Script"].Set(originalPropertiesRef["m_Script"].As<uint64_t>());
				YAML::Node scriptData = originalProperties["ScriptData"];
				for (YAML::const_iterator itor = scriptData.begin(); itor != scriptData.end(); ++itor)
				{
					const std::string name = itor->first.as<std::string>();
					Utils::NodeValueRef prop = props["ScriptData"][name];
					if (!prop.IsMap())
					{
						finalProps[name].Set(prop.Node());
						continue;
					}

					Utils::NodeValueRef originalSceneUUD = prop["SceneUUID"];
					Utils::NodeValueRef originalObjectUUD = prop["ObjectUUID"];

					Utils::NodeValueRef sceneUUID = finalProps["ScriptData"][name]["SceneUUID"];
					Utils::NodeValueRef objectUUID = finalProps["ScriptData"][name]["ObjectUUID"];

					if (!originalSceneUUD.Exists() || !originalObjectUUD.Exists())
					{
						finalProps["ScriptData"][name].Set(prop.Node());
						continue;
					}

					sceneUUID.Set((uint64_t)GetUUID());
					const UUID uuid = originalObjectUUD.As<uint64_t>();
					UUID remapped;
					if (!remapper.Find(uuid, remapped))
						remapped = uuid;

					objectUUID.Set(remapped);
				}

				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, finalProperties);
			}

			pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
			++currentComponentIndex;
		}

		for (size_t i = 0; i < node.ChildCount(); ++i)
		{
			const PrefabNode& childNode = node.ChildNode(i);
			InstantiatePrefabNode(pObject, childNode, remapper);
		}

		return pObject;
	}
}