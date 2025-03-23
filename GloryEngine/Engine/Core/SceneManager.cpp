#include "SceneManager.h"

#include "EngineProfiler.h"
#include "GScene.h"
#include "PropertyFlags.h"
#include "Systems.h"
#include "Components.h"
#include "Engine.h"
#include "Serializers.h"

#include <Reflection.h>

namespace Glory
{
	SceneManager::SceneManager(Engine* pEngine) : m_pEngine(pEngine), m_ActiveSceneIndex(0),
		m_HoveringObjectSceneID(0), m_HoveringObjectID(0), m_pComponentTypesInstance(nullptr)
	{
	}

	SceneManager::~SceneManager()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_pExternalScenes.clear();
		m_ActiveSceneIndex = 0;
	}

	Engine* SceneManager::GetEngine()
	{
		return m_pEngine;
	}

	GScene* SceneManager::GetHoveringEntityScene()
	{
		return m_HoveringObjectSceneID ? GetOpenScene(m_HoveringObjectSceneID) : nullptr;
	}

	UUID SceneManager::GetHoveringEntityUUID() const
	{
		return m_HoveringObjectID;
	}

	const glm::vec3& SceneManager::GetHoveringPosition() const
	{
		return m_HoveringPos;
	}

	const glm::vec3& SceneManager::GetHoveringNormal() const
	{
		return m_HoveringNormal;
	}

	void SceneManager::SetHoveringObject(UUID sceneID, UUID objectID)
	{
		m_HoveringObjectSceneID = sceneID;
		m_HoveringObjectID = objectID;
	}

	void SceneManager::SetHoveringPosition(const glm::vec3& pos)
	{
		m_HoveringPos = pos;
	}

	void SceneManager::SetHoveringNormal(const glm::vec3& normal)
	{
		m_HoveringNormal = normal;
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

	size_t SceneManager::GetSceneIndex(UUID uuid) const
	{
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) {return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return m_pOpenScenes.size();
		return it - m_pOpenScenes.begin();
	}

	size_t SceneManager::ExternalSceneCount()
	{
		return m_pExternalScenes.size();
	}

	GScene* SceneManager::GetExternalScene(size_t index)
	{
		if (index >= m_pExternalScenes.size()) return nullptr;
		return m_pExternalScenes[index];
	}

	void SceneManager::MarkAllScenesForDestruct()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->MarkForDestruction(); });
	}

	GScene* SceneManager::GetActiveScene(bool force)
	{
		if (m_ActiveSceneIndex >= m_pOpenScenes.size())
		{
			if (!force) return nullptr;
			return NewScene();
		}
		return m_pOpenScenes[m_ActiveSceneIndex];
	}

	void SceneManager::SetActiveScene(GScene* pScene)
	{
		auto it = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pScene);
		if (it == m_pOpenScenes.end()) return;
		m_ActiveSceneIndex = it - m_pOpenScenes.begin();
		OnSetActiveScene(pScene);
	}

	Utils::ECS::ComponentTypes* SceneManager::ComponentTypesInstance() const
	{
		Utils::ECS::ComponentTypes::SetInstance(m_pComponentTypesInstance);
		return m_pComponentTypesInstance;
	}

	void SceneManager::Initialize()
	{
		m_pComponentTypesInstance = Utils::ECS::ComponentTypes::CreateInstance();

		/* Register component types */
		Reflect::RegisterEnum<CameraPerspective>();
		Reflect::RegisterType<MeshMaterial>();

		/* Register engine components */
		RegisterComponent<Transform>();
		RegisterComponent<LayerComponent>();
		RegisterComponent<CameraComponent>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<ModelRenderer>();
		RegisterComponent<LightComponent>();
		RegisterComponent<TextComponent>();
		const Utils::Reflect::FieldData* pTextField = TextComponent::GetTypeData()->GetFieldData("m_Text");
		const Utils::Reflect::FieldData* pColorField = TextComponent::GetTypeData()->GetFieldData("m_Color");
		Reflect::SetFieldFlags(pTextField, AreaText);
		Reflect::SetFieldFlags(pColorField, Color);

		pColorField = LightComponent::GetTypeData()->GetFieldData(0);
		Reflect::SetFieldFlags(pColorField, PropertyFlags::Color);

		/* Temporary components for testing */
		RegisterComponent<Spin>();
		RegisterComponent<LookAt>();

		/* Register serializers */
		m_pEngine->GetResourceTypes().RegisterResource<GScene>(".gscene");

		// Register Invocations
		// Transform
		m_pComponentTypesInstance->RegisterInvokaction<Transform>(Glory::Utils::ECS::InvocationType::Start, TransformSystem::OnStart);
		m_pComponentTypesInstance->RegisterInvokaction<Transform>(Glory::Utils::ECS::InvocationType::Update, TransformSystem::OnUpdate);

		// Camera
		m_pComponentTypesInstance->RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::OnAdd, CameraSystem::OnComponentAdded);
		m_pComponentTypesInstance->RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::OnRemove, CameraSystem::OnComponentRemoved);
		m_pComponentTypesInstance->RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::Update, CameraSystem::OnUpdate);
		m_pComponentTypesInstance->RegisterInvokaction<CameraComponent>(Glory::Utils::ECS::InvocationType::Draw, CameraSystem::OnDraw);

		// Light
		m_pComponentTypesInstance->RegisterInvokaction<LightComponent>(Glory::Utils::ECS::InvocationType::Draw, LightSystem::OnDraw);

		// LookAt
		m_pComponentTypesInstance->RegisterInvokaction<LookAt>(Glory::Utils::ECS::InvocationType::Update, LookAtSystem::OnUpdate);

		// MeshRenderer
		m_pComponentTypesInstance->RegisterInvokaction<MeshRenderer>(Glory::Utils::ECS::InvocationType::Draw, MeshRenderSystem::OnDraw);
		m_pComponentTypesInstance->RegisterReferencesCallback<MeshRenderer>(MeshRenderSystem::GetReferences);

		// Spin
		m_pComponentTypesInstance->RegisterInvokaction<Spin>(Glory::Utils::ECS::InvocationType::Update, SpinSystem::OnUpdate);

		/* Text Renderer */
		m_pComponentTypesInstance->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::Draw, TextSystem::OnDraw);
		m_pComponentTypesInstance->RegisterReferencesCallback<TextComponent>(TextSystem::GetReferences);

		OnInitialize();
	}

	void SceneManager::Cleanup()
	{
		CloseAllScenes();
		Utils::ECS::ComponentTypes::DestroyInstance();
		m_pComponentTypesInstance = nullptr;
		OnCleanup();
	}

	void SceneManager::Update()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "SceneManager::Tick" };
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [this](GScene* pScene) {
			if (pScene->m_MarkedForDestruct)
			{
				CloseScene(pScene->GetUUID());
				return;
			}
			pScene->OnTick();
		});
		std::for_each(m_pExternalScenes.begin(), m_pExternalScenes.end(), [this](GScene* pScene) {
			pScene->OnTick();
		});
	}

	void SceneManager::Draw()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "SceneManager::Paint" };
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		std::for_each(m_pExternalScenes.begin(), m_pExternalScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
	}

	void SceneManager::Start()
	{
		for (size_t i = 0; i < m_pOpenScenes.size(); ++i)
		{
			m_pOpenScenes[i]->Start();
		}
	}

	void SceneManager::Stop()
	{
		for (size_t i = 0; i < m_pOpenScenes.size(); ++i)
		{
			m_pOpenScenes[i]->Stop();
		}
	}

	void SceneManager::AddExternalScene(GScene* pScene)
	{
		m_pExternalScenes.push_back(pScene);
		pScene->m_pManager = this;
	}

	void SceneManager::RemoveExternalScene(GScene* pScene)
	{
		auto iter = std::find(m_pExternalScenes.begin(), m_pExternalScenes.end(), pScene);
		if (iter == m_pExternalScenes.end()) return;
		m_pExternalScenes.erase(iter);
	}

	UUID SceneManager::AddSceneClosingCallback(std::function<void(UUID, UUID)> callback)
	{
		const UUID id = UUID();
		m_SceneClosedCallbacks.push_back({ id, callback });
		return id;
	}

	void SceneManager::RemoveSceneClosingCallback(UUID id)
	{
		auto iter = std::find_if(m_SceneClosedCallbacks.begin(), m_SceneClosedCallbacks.end(), [id](const SceneCallback& callback) {
			return callback.m_CallbackID == id;
		});
		if (iter == m_SceneClosedCallbacks.end()) return;
		m_SceneClosedCallbacks.erase(iter);
	}

	UUID SceneManager::AddSceneObjectDestroyedCallback(std::function<void(UUID, UUID)> callback)
	{
		const UUID id = UUID();
		m_SceneObjectDestroyedCallbacks.push_back({ id, callback });
		return id;
	}

	void SceneManager::RemoveSceneObjectDestroyedCallback(UUID id)
	{
		auto iter = std::find_if(m_SceneObjectDestroyedCallbacks.begin(), m_SceneObjectDestroyedCallbacks.end(), [id](const SceneCallback& callback) {
			return callback.m_CallbackID == id;
		});
		if (iter == m_SceneObjectDestroyedCallbacks.end()) return;
		m_SceneObjectDestroyedCallbacks.erase(iter);
	}

	void SceneManager::OnSceneObjectDestroyed(UUID objectID, UUID sceneID)
	{
		for (auto& callback : m_SceneObjectDestroyedCallbacks)
		{
			callback.m_Callback(sceneID, objectID);
		}
	}
	
	void SceneManager::SubscribeOnCopy(uint32_t hash, std::function<void(GScene*, void*, UUID, UUIDRemapper&)> callback)
	{
		m_OnComponentCopyCallbacks.emplace(hash, callback);
	}

	void SceneManager::TriggerOnCopy(uint32_t hash, GScene* pScene, void* data, UUID componentID, UUIDRemapper& remapper)
	{
		auto iter = m_OnComponentCopyCallbacks.find(hash);
		if (iter == m_OnComponentCopyCallbacks.end()) return;
		iter->second(pScene, data, componentID, remapper);
	}

	void SceneManager::OnSceneClosing(UUID sceneID)
	{
		for (auto& callback : m_SceneClosedCallbacks)
		{
			callback.m_Callback(sceneID, 0);
		}
	}

	void SceneManager::CloseAllScenes()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;
		OnCloseAll();
	}

	void SceneManager::UpdateScene(GScene* pScene) const
	{
		pScene->OnTick();
	}

	void SceneManager::DrawScene(GScene* pScene) const
	{
		pScene->OnPaint();
	}
}
