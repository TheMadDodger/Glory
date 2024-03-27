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

	void SceneManager::SetHoveringObject(UUID sceneID, UUID objectID)
	{
		m_HoveringObjectSceneID = sceneID;
		m_HoveringObjectID = objectID;
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
		RegisterComponent<MeshFilter>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<ModelRenderer>();
		RegisterComponent<LightComponent>();

		const FieldData* pColorField = LightComponent::GetTypeData()->GetFieldData(0);
		Reflect::SetFieldFlags(pColorField, Vec4Flags::Color);

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

		// Spin
		m_pComponentTypesInstance->RegisterInvokaction<Spin>(Glory::Utils::ECS::InvocationType::Update, SpinSystem::OnUpdate);

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
	}

	void SceneManager::Draw()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "SceneManager::Paint" };
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
	}

	void SceneManager::Start()
	{
		if (m_Started) return;
		m_Started = true;
		for (size_t i = 0; i < m_pOpenScenes.size(); ++i)
		{
			m_pOpenScenes[i]->Start();
		}
	}

	void SceneManager::CloseAllScenes()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;
		OnCloseAll();
	}
}
