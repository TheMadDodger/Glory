#include "CameraSystem.h"
#include <EntityRegistry.h>
#include <Window.h>
#include <Game.h>
#include <CameraManager.h>
#include <GloryContext.h>
#include <EntityRegistry.h>

namespace Glory
{
	CameraSystem::CameraSystem()
	{
	}

	CameraSystem::~CameraSystem()
	{
	}

	void CameraSystem::OnComponentAdded(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Engine* pEngine = Game::GetGame().GetEngine();
		Window* pWindow = pEngine->GetWindowModule()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);

		pComponent.m_Camera = GloryContext::GetCameraManager()->GetNewOrUnusedCamera();
		pComponent.m_Camera.SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
		pComponent.m_LastHash = CalcHash(pComponent);
	}

	void CameraSystem::OnComponentRemoved(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		pComponent.m_Camera.Free();
	}

	void CameraSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.SetView(glm::inverse(transform.MatTransform));

		uint32_t hash = CalcHash(pComponent);
		if (pComponent.m_LastHash == hash) return;
		pComponent.m_LastHash = hash;

		pComponent.m_Camera.SetDisplayIndex(pComponent.m_DisplayIndex);
		pComponent.m_Camera.SetPriority(pComponent.m_Priority);
		pComponent.m_Camera.SetLayerMask(pComponent.m_LayerMask);
		pComponent.m_Camera.SetClearColor(pComponent.m_ClearColor);

		Engine* pEngine = Game::GetGame().GetEngine();
		Window* pWindow = pEngine->GetWindowModule()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);
		pComponent.m_Camera.SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
	}

	void CameraSystem::OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pComponent.m_Camera);
	}

	std::string CameraSystem::Name()
	{
		return "Camera";
	}

	uint32_t CameraSystem::CalcHash(CameraComponent& pComponent)
	{
		float value = (float)pComponent.m_ClearColor.x + (float)pComponent.m_ClearColor.y
			+ (float)pComponent.m_ClearColor.z + (float)pComponent.m_ClearColor.w
			+ (float)pComponent.m_DisplayIndex + (float)pComponent.m_Far
			+ (float)pComponent.m_Near + (float)pComponent.m_HalfFOV
			+ (float)pComponent.m_Priority + (float)pComponent.m_LayerMask;

		return (uint32_t)std::hash<float>()(value);
	}
}
