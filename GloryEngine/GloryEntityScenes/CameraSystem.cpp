#include "CameraSystem.h"
#include <Window.h>
#include <Game.h>
#include "Registry.h"

namespace Glory
{
	void CameraSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, Camera& pComponent)
	{
		Engine* pEngine = Game::GetGame().GetEngine();
		Window* pWindow = pEngine->GetWindowModule()->GetMainWindow();

		int width, height;
		pWindow->GetDrawableSize(&width, &height);

		pComponent.m_Camera.m_Projection = glm::perspective(glm::radians(pComponent.m_HalfFOV), (float)width / (float)height, pComponent.m_Near, pComponent.m_Far);
	}

	void CameraSystem::OnUpdate(Registry* pRegistry, EntityID entity, Camera& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.m_View = transform.MatTransform;
	}

	void CameraSystem::OnDraw(Registry* pRegistry, EntityID entity, Camera& pComponent)
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pComponent.m_Camera);
	}
}
