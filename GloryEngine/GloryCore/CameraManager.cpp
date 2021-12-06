#include "CameraManager.h"

namespace Glory
{
	std::vector<Camera> CameraManager::m_Cameras;
	std::vector<size_t> CameraManager::m_UnusedCameraIndices;
	std::unordered_map<UUID, size_t> CameraManager::m_IDToCamera;

	Camera* CameraManager::GetNewOrUnusedCamera()
	{
		if (m_UnusedCameraIndices.size() > 0)
		{
			size_t index = m_UnusedCameraIndices[0];
			m_UnusedCameraIndices.erase(m_UnusedCameraIndices.begin());
			Camera* pCamera = &m_Cameras[index];
			pCamera->m_IsInUse = true;
			return pCamera;
		}

		size_t index = m_Cameras.size();
		m_Cameras.push_back(Camera());
		UUID id = m_Cameras[index].GetUUID();
		m_IDToCamera[id] = index;
		return &m_Cameras[index];
	}

	void CameraManager::SetUnused(Camera* pCamera)
	{
		size_t index = m_IDToCamera[pCamera->GetUUID()];
		m_UnusedCameraIndices.push_back(index);
		pCamera->m_IsInUse = false;
	}

	RenderTexture* CameraManager::GetRenderTextureForCamera(Camera* pCamera, Engine* pEngine)
	{
		if (pCamera->m_pRenderTexture) return pCamera->m_pRenderTexture;

		Window* pWindow = pEngine->GetWindowModule()->GetMainWindow();
		int width, height;
		pWindow->GetDrawableSize(&width, &height);
		GPUResourceManager* pResourceManager = pEngine->GetGraphicsModule()->GetResourceManager();
		pCamera->m_pRenderTexture = pResourceManager->CreateRenderTexture(width, height, true);
		return pCamera->m_pRenderTexture;
	}

	CameraManager::CameraManager() {}

	CameraManager::~CameraManager() {}
}
