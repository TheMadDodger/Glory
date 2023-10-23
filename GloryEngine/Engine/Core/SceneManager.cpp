#include "SceneManager.h"
#include "Engine.h"
#include "GScene.h"
#include "EngineProfiler.h"
#include "Serializer.h"
#include "Components.h"
#include "PropertyFlags.h"
#include "PropertySerializer.h"
#include "SceneSerializer.h"
#include "SceneObjectSerializer.h"
#include "ScriptedComponentSerializer.h"
#include "PhysicsModule.h"
#include "PhysicsSystem.h"
#include "PrefabDataLoader.h"
#include "SceneObject.h"

#include <GloryECS/ComponentTypes.h>

namespace Glory
{
	SceneManager::SceneManager(Engine* pEngine) : m_pEngine(pEngine), m_ActiveSceneIndex(0)
	{
	}

	SceneManager::~SceneManager()
	{
	}

	GScene* SceneManager::CreateEmptyScene(const std::string& name)
	{
		Profiler::BeginSample("ScenesModule::CreateEmptyScene");
		GScene* pScene = new GScene(name);
		pScene->Initialize();
		m_pOpenScenes.push_back(pScene);
		Profiler::EndSample();
		return pScene;
	}

	size_t SceneManager::OpenScenesCount()
	{
		return m_pOpenScenes.size();
	}

	GScene* SceneManager::GetOpenScene(size_t index)
	{
		if (index >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[index];
	}

	GScene* SceneManager::GetOpenScene(UUID uuid)
	{
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) {return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return nullptr;
		return *it;
	}

	GScene* SceneManager::GetActiveScene()
	{
		if (m_ActiveSceneIndex >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[m_ActiveSceneIndex];
	}

	void SceneManager::SetActiveScene(GScene* pScene)
	{
		auto it = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pScene);
		if (it == m_pOpenScenes.end()) return;
		m_ActiveSceneIndex = it - m_pOpenScenes.begin();
	}

	void SceneManager::CloseAllScenes()
	{
		Cleanup();
	}

	void SceneManager::OpenScene(const std::string& path, UUID uuid)
	{
		YAML::Node node = YAML::LoadFile(path);
		std::filesystem::path filePath = path;
		GScene* pScene = Serializer::DeserializeObjectOfType<GScene>(node, filePath.filename().replace_extension().string());
		if (pScene == nullptr) return;
		pScene->SetUUID(uuid);
		m_pOpenScenes.push_back(pScene);
		OnSceneOpen(uuid);
	}

	void SceneManager::AddOpenScene(GScene* pScene, UUID uuid)
	{
		if (pScene == nullptr) return;
		if (uuid) pScene->SetUUID(uuid);
		m_pOpenScenes.push_back(pScene);
		OnSceneOpen(uuid);
	}

	void SceneManager::CloseScene(UUID uuid)
	{
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) { return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return;
		size_t index = it - m_pOpenScenes.begin();
		GScene* pActiveScene = m_pOpenScenes[m_ActiveSceneIndex];

		OnSceneClose(uuid);
		GScene* pScene = *it;
		delete pScene;
		m_pOpenScenes.erase(it);

		if (index == m_ActiveSceneIndex || m_pOpenScenes.size() <= 0)
		{
			m_ActiveSceneIndex = 0;
			return;
		}

		it = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pActiveScene);
		m_ActiveSceneIndex = it - m_pOpenScenes.begin();
	}

	void SceneManager::SetHoveringObject(uint64_t objectID)
	{
		std::unique_lock<std::mutex> lock(m_HoveringLock);
		m_pHoveringObject = GetSceneObjectFromObjectID(objectID);
		lock.unlock();
	}

	SceneObject* SceneManager::GetHoveringObject()
	{
		SceneObject* pObject = nullptr;
		std::unique_lock<std::mutex> lock(m_HoveringLock);
		pObject = m_pHoveringObject;
		lock.unlock();
		return pObject;
	}

	SceneObject* SceneManager::GetSceneObjectFromObjectID(uint64_t objectID)
	{
		for (size_t i = 0; i < m_pOpenScenes.size(); i++)
		{
			GScene* pScene = m_pOpenScenes[i];
			SceneObject* pEntityObject = pScene->GetSceneObjectFromEntityID(objectID);
			if (pEntityObject) return pEntityObject;
		}

		return nullptr;
	}

	Utils::ECS::ComponentTypes* SceneManager::ComponentTypesInstance() const
	{
		return m_pComponentTypesInstance;
	}

	void SceneManager::Initialize()
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
		Serializer::RegisterSerializer<SceneSerializer>();
		Serializer::RegisterSerializer<SceneObjectSerializer>();
		PropertySerializer::RegisterSerializer<ScriptedComponentSerailizer>();
		ResourceType::RegisterResource<GScene>(".gscene");

		PhysicsModule* pPhysics = m_pEngine->GetMainModule<PhysicsModule>();
		if (!pPhysics) return;

		pPhysics->RegisterActivationCallback(ActivationCallback::Activated, PhysicsSystem::OnBodyActivated);
		pPhysics->RegisterActivationCallback(ActivationCallback::Deactivated, PhysicsSystem::OnBodyDeactivated);

		pPhysics->RegisterContactCallback(ContactCallback::Added, PhysicsSystem::OnContactAdded);
		pPhysics->RegisterContactCallback(ContactCallback::Persisted, PhysicsSystem::OnContactPersisted);
		pPhysics->RegisterContactCallback(ContactCallback::Removed, PhysicsSystem::OnContactRemoved);

		m_pPrefabLoader = new PrefabDataLoader();
		m_pEngine->AddOptionalModule(m_pPrefabLoader, true);
	}

	void SceneManager::Cleanup()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;

		Utils::ECS::ComponentTypes::DestroyInstance();
		m_pComponentTypesInstance = nullptr;
	}

	void SceneManager::Update()
	{
		Profiler::BeginSample("SceneManager::Tick");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
		Profiler::EndSample();
	}

	void SceneManager::Draw()
	{
		Profiler::BeginSample("SceneManager::Paint");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		Profiler::EndSample();
	}

	SceneObject* SceneManager::CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return CreateObject(pScene, name, uuid);
	}

	SceneObject* SceneManager::CreateObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return pScene->CreateObject(name, uuid);
	}
}
