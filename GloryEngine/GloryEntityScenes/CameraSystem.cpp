#include "CameraSystem.h"
#include "Registry.h"
#include <Window.h>
#include <Game.h>
#include <CameraManager.h>
#include <SerializedPropertyManager.h>

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
		pComponent.m_LastHash = CalcHash(pComponent);
	}

	void CameraSystem::OnComponentRemoved(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		pComponent.m_Camera.Free();
	}

	void CameraSystem::OnUpdate(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Camera.SetView(transform.MatTransform);

		size_t hash = CalcHash(pComponent);
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

	void CameraSystem::OnDraw(Registry* pRegistry, EntityID entity, CameraComponent& pComponent)
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pComponent.m_Camera);
	}

	void CameraSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, CameraComponent& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Half FOV"), 0, &pComponent.m_HalfFOV));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Near"), 0, &pComponent.m_Near));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Far"), 0, &pComponent.m_Far));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<int>>(uuid, std::string("Display Index"), 0, &pComponent.m_DisplayIndex));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<int>>(uuid, std::string("Priority"), 0, &pComponent.m_Priority));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<LayerMask>>(uuid, std::string("Layer Mask"), 0, &pComponent.m_LayerMask));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec4>>(uuid, std::string("Clear Color"), 0, &pComponent.m_ClearColor));
	}

	std::string CameraSystem::Name()
	{
		return "Camera";
	}

	size_t CameraSystem::CalcHash(CameraComponent& pComponent)
	{
		float value = (float)pComponent.m_ClearColor.x + (float)pComponent.m_ClearColor.y
			+ (float)pComponent.m_ClearColor.z + (float)pComponent.m_ClearColor.w
			+ (float)pComponent.m_DisplayIndex + (float)pComponent.m_Far
			+ (float)pComponent.m_Near + (float)pComponent.m_HalfFOV
			+ (float)pComponent.m_Priority + (float)pComponent.m_LayerMask;

		return std::hash<float>()(value);
	}
}
