#include <glm/glm.hpp>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include "EntitySceneObject.h"
#include "EntityPrefabData.h"
#include "PropertySerializer.h"

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

	GLORY_API bool EntityScene::IsValid() const
	{
		return m_Valid;
	}

	SceneObject* EntityScene::InstantiatePrefab(const std::map<UUID, UUID>& idRemap, EntityPrefabData* pPrefab)
	{
		const PrefabNode& rootNode = pPrefab->RootNode();
		EntitySceneObject* pInstantiatedPrefab = InstantiatePrefabNode(idRemap, nullptr, rootNode);
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

	EntitySceneObject* EntityScene::InstantiatePrefabNode(const std::map<UUID, UUID>& idRemap, EntitySceneObject* pParent, const PrefabNode& node)
	{
		auto itor = idRemap.find(node.OriginalUUID());
		const UUID objectID = itor != idRemap.end() ? itor->second : UUID();
		itor = idRemap.find(node.TransformUUID());
		const UUID transformID = itor != idRemap.end() ? itor->second : UUID();
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

			itor = idRemap.find(originalUUID);
			UUID compUUID = itor != idRemap.end() ? itor->second : UUID();

			void* pComponentAddress = nullptr;
			if (typeHash != transformTypeHash) pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
			else
			{
				Utils::ECS::EntityView * pEntityView = pRegistry->GetEntityView(entity);
				compUUID = pEntityView->ComponentUUIDAt(0);
				pComponentAddress = pRegistry->GetComponentAddress(entity, compUUID);
			}

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
			YAML::Node originalProperties = nextObject["Properties"];
			if (typeHash != scriptedTypeHash)
			{
				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, originalProperties);
			}
			else
			{
				YAML::Node finalProperties = YAML::Node(YAML::NodeType::Map);

				NodeRef originalPropertiesRef = originalProperties;
				NodeRef finalPropertiesRef = finalProperties;

				NodeValueRef props = originalPropertiesRef.ValueRef();
				NodeValueRef finalProps = finalPropertiesRef.ValueRef();

				finalPropertiesRef["m_Script"].Set(originalPropertiesRef["m_Script"].As<uint64_t>());
				YAML::Node scriptData = originalProperties["ScriptData"];
				for (YAML::const_iterator itor = scriptData.begin(); itor != scriptData.end(); ++itor)
				{
					const std::string name = itor->first.as<std::string>();
					NodeValueRef prop = props["ScriptData"][name];
					if (!prop.IsMap())
					{
						finalProps[name].Set(prop.Node());
						continue;
					}

					NodeValueRef originalSceneUUD = prop["SceneUUID"];
					NodeValueRef originalObjectUUD = prop["ObjectUUID"];

					NodeValueRef sceneUUID = finalProps["ScriptData"][name]["SceneUUID"];
					NodeValueRef objectUUID = finalProps["ScriptData"][name]["ObjectUUID"];

					if (!originalSceneUUD.Exists() || !originalObjectUUD.Exists())
					{
						finalProps["ScriptData"][name].Set(prop.Node());
						continue;
					}

					sceneUUID.Set((uint64_t)GetUUID());
					const UUID uuid = originalObjectUUD.As<uint64_t>();
					auto idItor = idRemap.find(uuid);
					if (idItor == idRemap.end())
					{
						objectUUID.Set(originalObjectUUD.As<uint64_t>());
						continue;
					}
					objectUUID.Set((uint64_t)idItor->second);
				}

				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, finalProperties);
			}

			pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
			++currentComponentIndex;
		}

		for (size_t i = 0; i < node.ChildCount(); ++i)
		{
			const PrefabNode& childNode = node.ChildNode(i);
			InstantiatePrefabNode(idRemap, pObject, childNode);
		}

		return pObject;
	}
}