#include <glm/glm.hpp>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include "EntitySceneObject.h"

namespace Glory
{
	EntityScene::EntityScene() : m_Valid(true), GScene("New Scene") //: m_pJobPool(nullptr)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName) : m_Valid(true), GScene(sceneName)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName, UUID uuid) : m_Valid(true), GScene(sceneName, uuid)
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

	void EntityScene::Initialize()
	{
		// Register Invocations
		// Camera
		m_Registry.RegisterInvokaction<CameraComponent>(GloryECS::InvocationType::OnAdd, CameraSystem::OnComponentAdded);
		m_Registry.RegisterInvokaction<CameraComponent>(GloryECS::InvocationType::OnRemove, CameraSystem::OnComponentRemoved);
		m_Registry.RegisterInvokaction<CameraComponent>(GloryECS::InvocationType::Update, CameraSystem::OnUpdate);
		m_Registry.RegisterInvokaction<CameraComponent>(GloryECS::InvocationType::Draw, CameraSystem::OnDraw);

		// Light
		m_Registry.RegisterInvokaction<LightComponent>(GloryECS::InvocationType::Draw, LightSystem::OnDraw);

		// LookAt
		m_Registry.RegisterInvokaction<LookAt>(GloryECS::InvocationType::Update, LookAtSystem::OnUpdate);

		// MeshRenderer
		m_Registry.RegisterInvokaction<MeshRenderer>(GloryECS::InvocationType::Draw, MeshRenderSystem::OnDraw);

		// Spin
		m_Registry.RegisterInvokaction<Spin>(GloryECS::InvocationType::Update, SpinSystem::OnUpdate);

		// Transform
		m_Registry.RegisterInvokaction<Transform>(GloryECS::InvocationType::Update, TransformSystem::OnUpdate);

		// Scripted
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::OnAdd, ScriptedSystem::OnAdd);
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::Update, ScriptedSystem::OnUpdate);
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::Draw, ScriptedSystem::OnDraw);
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::Start, ScriptedSystem::OnStart);
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::Stop, ScriptedSystem::OnStop);
		m_Registry.RegisterInvokaction<ScriptedComponent>(GloryECS::InvocationType::OnValidate, ScriptedSystem::OnValidate);

		// TEMPORARY
		m_Registry.RegisterInvokaction<LayerComponent>(GloryECS::InvocationType::OnAdd, [](EntityRegistry*, EntityID, LayerComponent&) {});
	}

	void EntityScene::OnTick()
	{
		m_Registry.InvokeAll(GloryECS::InvocationType::Update);
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntityScene::OnPaint()
	{
		m_Registry.InvokeAll(GloryECS::InvocationType::Draw);
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
}