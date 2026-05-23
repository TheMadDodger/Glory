#include "SceneManager.h"

#include "EngineProfiler.h"
#include "GScene.h"
#include "PropertyFlags.h"
#include "Components.h"
#include "IEngine.h"
#include "Renderer.h"

#include "TransformManager.h"
#include "MeshRenderManager.h"
#include "CameraComponentManager.h"
#include "LightManager.h"
#include "TextManager.h"
#include "Resources.h"

#include <Reflection.h>

namespace Glory
{
	SceneManager::SceneManager(IEngine* pEngine) : m_pEngine(pEngine), m_pRenderer(nullptr), m_ActiveSceneIndex(0),
		m_HoveringObjectSceneID(0), m_HoveringObjectID(0), m_HoveringPos(),
		m_HoveringNormal(), m_NextFrameLoadIsAdditive(false)
	{
	}

	SceneManager::~SceneManager()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_pExternalScenes.clear();
		m_ActiveSceneIndex = 0;

		m_pRenderer = nullptr;
	}

	void SceneManager::SetRenderer(Renderer* pRenderer)
	{
		if (m_pRenderer) m_pRenderer->SetSceneManager(nullptr);
		m_pRenderer = pRenderer;
		if (pRenderer) pRenderer->SetSceneManager(this);
	}

	Renderer* SceneManager::GetRenderer() const
	{
		return m_pRenderer;
	}

	void SceneManager::LoadScene(UUID uuid, bool additive)
	{
		if (!additive)
			UnloadAllScenes();
		else if (GetOpenScene(uuid))
			UnloadScene(uuid);

		OnLoadScene(uuid);
	}

	void SceneManager::UnloadScene(UUID uuid)
	{
		GScene* pActiveScene = GetActiveScene();
		const size_t index = GetSceneIndex(uuid);
		if (index == m_pOpenScenes.size()) return;
		GScene* pScene = m_pOpenScenes[index];
		pScene->Stop();
		OnUnloadScene(pScene);
		pScene->m_MarkedForDestruct = true;
		m_pRemovedScenes.push_back(pScene);
		m_pOpenScenes.erase(m_pOpenScenes.begin() + index);

		if (m_pOpenScenes.empty() || pActiveScene == pScene)
			m_ActiveSceneIndex = 0;
		else
		{
			auto iter = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pActiveScene);
			m_ActiveSceneIndex = iter - m_pOpenScenes.begin();
		}

		for (auto& callback : m_SceneClosedCallbacks)
		{
			callback.m_Callback(uuid, 0);
		}
	}

	void SceneManager::UnloadAllScenes()
	{
		for (size_t i = 0; i < m_pOpenScenes.size(); ++i)
		{
			GScene* pScene = m_pOpenScenes[i];
			pScene->Stop();
			OnUnloadScene(pScene);
			pScene->m_MarkedForDestruct = true;
			pScene->m_Registry.DisableCalls();
			m_pRemovedScenes.push_back(pScene);
		}
		OnUnloadAllScenes();
		m_pOpenScenes.clear();
	}

	void SceneManager::LoadSceneNextFrame(UUID uuid, bool additive)
	{
		if (!additive)
			m_ToLoadNextFrame.clear();
		m_ToLoadNextFrame.push_back(uuid);
		m_NextFrameLoadIsAdditive = additive;
	}

	IEngine* SceneManager::GetEngine()
	{
		return m_pEngine;
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

	Utils::ECS::RegistryFactory& SceneManager::GetRegistryFactory()
	{
		return m_RegistryFactory;
	}

	void SceneManager::Initialize()
	{
		/* Register engine component managers */
		RegisterComponentManagers();

		const Utils::Reflect::FieldData* pTextField = TextComponent::GetTypeData()->GetFieldData("m_Text");
		const Utils::Reflect::FieldData* pColorField = TextComponent::GetTypeData()->GetFieldData("m_Color");
		Reflect::SetFieldFlags(pTextField, AreaText);
		Reflect::SetFieldFlags(pColorField, Color);

		pColorField = LightComponent::GetTypeData()->GetFieldData(0);
		Reflect::SetFieldFlags(pColorField, PropertyFlags::Color);

		m_pEngine->GetResourceTypes().RegisterResource<GScene>("", [this](GScene* pScene) {
			m_RegistryFactory.PopulateRegisry(pScene->GetRegistry());
		});

		OnInitialize();
	}

	void SceneManager::Cleanup()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		std::for_each(m_pRemovedScenes.begin(), m_pRemovedScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pRemovedScenes.clear();
		m_pExternalScenes.clear();
		m_ActiveSceneIndex = 0;
		OnCleanup();
	}

	void SceneManager::Update(float dt)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "SceneManager::Tick" };
		std::for_each(m_ToLoadNextFrame.begin(), m_ToLoadNextFrame.end(), [this](const UUID sceneID) {
			LoadScene(sceneID, m_NextFrameLoadIsAdditive);
		});
		m_ToLoadNextFrame.clear();

		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [dt](GScene* pScene) {
			pScene->OnTick(dt);
		});
		std::for_each(m_pExternalScenes.begin(), m_pExternalScenes.end(), [dt](GScene* pScene) {
			pScene->OnTick(dt);
		});

		std::for_each(m_pRemovedScenes.begin(), m_pRemovedScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pRemovedScenes.clear();
	}

	void SceneManager::Draw()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "SceneManager::Paint" };
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		std::for_each(m_pExternalScenes.begin(), m_pExternalScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
	}

	void SceneManager::RegisterComponentManagers()
	{
		RegisterComponentManager<TransformManager, Transform>();
		RegisterComponentManager<Utils::ECS::ComponentManager<LayerComponent>, LayerComponent>();
		RegisterComponentManager<CameraComponentManager, CameraComponent>(
			[this](Utils::ECS::EntityRegistry*, CameraComponentManager* manager) {
				manager->m_pSceneManager = this;
				manager->m_pCameraManager = &m_pEngine->GetCameraManager();
			});
		RegisterComponentManager<MeshRenderManager, MeshRenderer>(
			[this](Utils::ECS::EntityRegistry*, MeshRenderManager* manager) {
				manager->m_pSceneManager = this;
				manager->m_pResources = &m_pEngine->GetResources();
				manager->m_pMaterialManager = &m_pEngine->GetMaterialManager();
				manager->m_pAssetDatabase = &m_pEngine->GetAssetDatabase();
				manager->m_pLayerManager = &m_pEngine->GetLayerManager();
				manager->m_pDebug = &m_pEngine->GetDebug();
			});
		RegisterComponentManager<LightManager, LightComponent>(
			[this](Utils::ECS::EntityRegistry*, LightManager* manager) {
				manager->m_pSceneManager = this;
			});
		RegisterComponentManager<TextManager, TextComponent>(
			[this](Utils::ECS::EntityRegistry*, TextManager* manager) {
				manager->m_pSceneManager = this;
				manager->m_pResources = &m_pEngine->GetResources();
				manager->m_pLayerManager = &m_pEngine->GetLayerManager();
			});
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
		m_RegistryFactory.PopulateRegisry(pScene->GetRegistry());
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

	void SceneManager::UpdateScene(GScene* pScene, float dt) const
	{
		if (pScene->m_MarkedForDestruct) return;
		pScene->OnTick(dt);
	}

	void SceneManager::DrawScene(GScene* pScene) const
	{
		if (pScene->m_MarkedForDestruct) return;
		pScene->OnPaint();
	}

	GScene* SceneManager::CreateNewScene_Internal(const std::string& name, UUID uuid)
	{
		GScene* pNewScene = new GScene(name, uuid);
		m_RegistryFactory.PopulateRegisry(pNewScene->m_Registry);
		pNewScene->m_pManager = this;
		return pNewScene;
	}
}
