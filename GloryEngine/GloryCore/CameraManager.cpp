#include "CameraManager.h"
#include "Game.h"
#include "Engine.h"
#include "WindowModule.h"
#include "EngineProfiler.h"

namespace Glory
{
	std::vector<Camera> CameraManager::m_Cameras;
	std::vector<size_t> CameraManager::m_UnusedCameraIndices;
	std::unordered_map<UUID, size_t> CameraManager::m_IDToCamera;

	CameraRef CameraManager::GetNewOrUnusedCamera()
	{
		Profiler::BeginSample("CameraManager::GetNewOrUnusedCamera");
		int width, height;
		Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow()->GetDrawableSize(&width, &height);

		if (m_UnusedCameraIndices.size() > 0)
		{
			size_t index = m_UnusedCameraIndices[0];
			m_UnusedCameraIndices.erase(m_UnusedCameraIndices.begin());
			Camera& pCamera = m_Cameras[index];
			pCamera.SetResolution(width, height);
			pCamera.m_IsInUse = true;
			Profiler::EndSample();
			return CameraRef(pCamera.GetUUID());
		}

		size_t index = m_Cameras.size();
		m_Cameras.push_back(Camera(width, height));
		UUID id = m_Cameras[index].GetUUID();
		m_IDToCamera[id] = index;
		Profiler::EndSample();
		return CameraRef(m_Cameras[index].GetUUID());
	}

	void CameraManager::SetUnused(Camera* pCamera)
	{
		size_t index = m_IDToCamera[pCamera->GetUUID()];
		m_UnusedCameraIndices.push_back(index);
		pCamera->m_IsInUse = false;
	}

	RenderTexture* CameraManager::GetRenderTextureForCamera(CameraRef camera, Engine* pEngine)
	{
		Profiler::BeginSample("CameraManager::GetRenderTextureForCamera");
		Camera* pCamera = GetCamera(camera.m_CameraID);
		if (pCamera == nullptr) return nullptr;
		if (pCamera->m_pRenderTexture)
		{
			if (pCamera->m_TextureIsDirty)
			{
				pCamera->m_pRenderTexture->Resize(pCamera->m_Resolution.x, pCamera->m_Resolution.y);
				pCamera->m_TextureIsDirty = false;
			}
			Profiler::EndSample();
			return pCamera->m_pRenderTexture;
		}

		int width = pCamera->m_Resolution.x;
		int height = pCamera->m_Resolution.y;
		GPUResourceManager* pResourceManager = pEngine->GetGraphicsModule()->GetResourceManager();
		pCamera->m_pRenderTexture = pResourceManager->CreateRenderTexture(width, height, true);
		pCamera->m_TextureIsDirty = false;
		Profiler::EndSample();
		return pCamera->m_pRenderTexture;
	}

	Camera* CameraManager::GetCamera(UUID uuid)
	{
		if (m_IDToCamera.find(uuid) == m_IDToCamera.end()) return nullptr;
		size_t index = m_IDToCamera[uuid];
		return &m_Cameras[index];
	}

	CameraManager::CameraManager() {}

	CameraManager::~CameraManager() {}
}
