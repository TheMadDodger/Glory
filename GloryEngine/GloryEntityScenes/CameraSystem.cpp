#include "CameraSystem.h"
#include <Window.h>
#include <Game.h>
#include "Registry.h"
#include <CameraManager.h>

namespace Glory
{
	void CameraSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Engine* pEngine = Game::GetGame().GetEngine();
		Window* pWindow = pEngine->GetWindowModule()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);

		pComponent.m_pCamera = CameraManager::GetNewOrUnusedCamera();
		pComponent.m_pCamera->SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
	}

	void CameraSystem::OnComponentRemoved(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		if (pComponent.m_pCamera == nullptr) return;
		CameraManager::SetUnused(pComponent.m_pCamera);
	}

	void CameraSystem::OnUpdate(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		if (pComponent.m_pCamera == nullptr) return;
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_pCamera->SetView(transform.MatTransform);
	}

	void CameraSystem::OnDraw(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		if (pComponent.m_pCamera == nullptr) return;
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pComponent.m_pCamera);
	}
}
