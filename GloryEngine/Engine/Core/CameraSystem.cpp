#include "CameraSystem.h"

#include "Engine.h"
#include "WindowModule.h"
#include "RendererModule.h"
#include "Window.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "GScene.h"

#include <EntityRegistry.h>

namespace Glory
{
	CameraSystem::CameraSystem()
	{
	}

	CameraSystem::~CameraSystem()
	{
	}

	void CameraSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer) return;

		const glm::uvec2& resolution = pRenderer->Resolution();

		switch (pComponent.m_OutputMode)
		{
		case CameraOutputMode::None:
		case CameraOutputMode::FixedResolution:
			pComponent.m_Camera.SetBaseResolution(uint32_t(pComponent.m_Resolution.x), uint32_t(pComponent.m_Resolution.y));
			pComponent.m_Camera.SetResolutionScale(1.0f, 1.0f);
			break;
		case CameraOutputMode::ScaledResolution:
			pComponent.m_Camera.SetBaseResolution(resolution.x, resolution.y);
			pComponent.m_Camera.SetResolutionScale(pComponent.m_Resolution.x, pComponent.m_Resolution.y);
			break;
		default:
			break;
		}

		pComponent.m_Camera.SetOutput(pComponent.m_OutputMode != CameraOutputMode::None, int(pComponent.m_Offset.x), int(pComponent.m_Offset.y));
		pComponent.m_Camera.SetPerspectiveProjection(pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
		pComponent.m_Camera.SetClearColor(pComponent.m_ClearColor);
		pComponent.m_Camera.SetPriority(pComponent.m_Priority);
		pComponent.m_Camera.SetLayerMask(pComponent.m_LayerMask);

		pRenderer->UpdateCamera(pComponent.m_Camera);
	}

	void CameraSystem::OnComponentAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		Window* pWindow = pEngine->GetMainModule<WindowModule>()->GetMainWindow();
		pComponent.m_Camera = pEngine->GetCameraManager().GetNewOrUnusedCamera();
		OnValidate(pRegistry, entity, pComponent);
	}

	void CameraSystem::OnComponentRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		pComponent.m_Camera.Free();
		pComponent.m_Camera = NULL;
	}

	void CameraSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.SetView(glm::inverse(transform.MatTransform));

		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer || !pRenderer->ResolutionChanged()) return;
		OnValidate(pRegistry, entity, pComponent);
	}

	void CameraSystem::OnEnableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer) return;
		pRenderer->SubmitCamera(pComponent.m_Camera);
	}

	void CameraSystem::OnDisableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer) return;
		pRenderer->UnsubmitCamera(pComponent.m_Camera);
	}

	std::string CameraSystem::Name()
	{
		return "Camera";
	}
}
