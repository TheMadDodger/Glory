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

	void CameraSystem::OnComponentAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		Window* pWindow = pEngine->GetMainModule<WindowModule>()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);

		pComponent.m_Camera = pEngine->GetCameraManager().GetNewOrUnusedCamera();
		pComponent.m_Camera.SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
		pComponent.m_LastHash = CalcHash(pComponent);
	}

	void CameraSystem::OnComponentRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		pComponent.m_Camera.Free();
	}

	void CameraSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.SetView(glm::inverse(transform.MatTransform));

		uint32_t hash = CalcHash(pComponent);
		if (pComponent.m_LastHash == hash) return;
		pComponent.m_LastHash = hash;

		pComponent.m_Camera.SetPriority(pComponent.m_Priority);
		pComponent.m_Camera.SetLayerMask(pComponent.m_LayerMask);
		pComponent.m_Camera.SetClearColor(pComponent.m_ClearColor);

		Window* pWindow = pEngine->GetMainModule<WindowModule>()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);
		pComponent.m_Camera.SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
	}

	void CameraSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CameraComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		pEngine->GetMainModule<RendererModule>()->Submit(pComponent.m_Camera);
	}

	std::string CameraSystem::Name()
	{
		return "Camera";
	}

	uint32_t CameraSystem::CalcHash(CameraComponent& pComponent)
	{
		float value = (float)pComponent.m_ClearColor.x + (float)pComponent.m_ClearColor.y
			+ (float)pComponent.m_ClearColor.z + (float)pComponent.m_ClearColor.w
			+ (float)pComponent.m_Far + (float)pComponent.m_Near
			+ (float)pComponent.m_HalfFOV + (float)pComponent.m_Priority
			+ (float)pComponent.m_LayerMask;

		return (uint32_t)std::hash<float>()(value);
	}
}
