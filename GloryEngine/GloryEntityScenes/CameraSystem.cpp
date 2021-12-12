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

		pComponent.m_Camera = CameraManager::GetNewOrUnusedCamera();
		pComponent.m_Camera.SetPerspectiveProjection(width, height, pComponent.m_HalfFOV, pComponent.m_Near, pComponent.m_Far);
	}

	void CameraSystem::OnComponentRemoved(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		pComponent.m_Camera.Free();
	}

	void CameraSystem::OnUpdate(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.SetView(transform.MatTransform);
		pComponent.m_Camera.SetDisplayIndex(pComponent.m_DisplayIndex);
	}

	void CameraSystem::OnDraw(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pComponent.m_Camera);
	}
}
