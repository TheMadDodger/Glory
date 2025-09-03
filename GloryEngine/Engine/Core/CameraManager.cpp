#include "CameraManager.h"
#include "Engine.h"
#include "RendererModule.h"
#include "EngineProfiler.h"
#include "DisplayManager.h"
#include "Camera.h"

namespace Glory
{
	CameraRef CameraManager::GetNewOrUnusedCamera()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "CameraManager::GetNewOrUnusedCamera" };
		uint32_t width, height;
		m_pEngine->GetDisplayManager().GetResolution(width, height);

		if (m_UnusedCameraIndices.size() > 0)
		{
			size_t index = m_UnusedCameraIndices[0];
			m_UnusedCameraIndices.erase(m_UnusedCameraIndices.begin());
			Camera& pCamera = m_Cameras[index];
			pCamera.SetResolution(width, height);
			pCamera.m_IsInUse = true;
			return CameraRef(this, pCamera.GetUUID());
		}

		size_t index = m_Cameras.size();
		m_Cameras.push_back(Camera(width, height));
		UUID id = m_Cameras[index].GetUUID();
		m_IDToCamera[id] = index;
		return CameraRef(this, m_Cameras[index].GetUUID());
	}

	void CameraManager::SetUnused(Camera* pCamera)
	{
		size_t index = m_IDToCamera[pCamera->GetUUID()];
		m_UnusedCameraIndices.push_back(index);
		pCamera->m_IsInUse = false;
	}

	RenderTexture* CameraManager::GetRenderTextureForCamera(CameraRef camera, Engine* pEngine, size_t index, bool createIfNotExist)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "CameraManager::GetNewOrUnusedCamera" };
		Camera* pCamera = GetCamera(camera.m_CameraID);
		if (pCamera == nullptr) return nullptr;
		if (pCamera->m_pRenderTextures.size() > index)
		{
			if (createIfNotExist && pCamera->m_TextureIsDirty)
			{
				for (size_t i = 0; i < pCamera->m_pRenderTextures.size(); ++i)
				{
					pCamera->m_pRenderTextures[i]->Resize(pCamera->m_Resolution.x, pCamera->m_Resolution.y);
				}

				pCamera->m_TextureIsDirty = false;
				pCamera->m_PerspectiveDirty = false;
				pEngine->GetMainModule<RendererModule>()->OnCameraResize(camera);
			}
			else if (createIfNotExist && pCamera->m_PerspectiveDirty)
			{
				pCamera->m_PerspectiveDirty = false;
				pEngine->GetMainModule<RendererModule>()->OnCameraPerspectiveChanged(camera);
			}
			return pCamera->m_pRenderTextures[index];
		}

		if (!createIfNotExist) return nullptr;
		pCamera->m_TextureIsDirty = false;
		return pCamera->m_pRenderTextures[index];
	}

	size_t CameraManager::CameraRenderTextureCount(CameraRef camera, Engine* pEngine)
	{
		Camera* pCamera = GetCamera(camera.m_CameraID);
		if (!pCamera) return 0;
		return pCamera->m_pRenderTextures.size();
	}

	Camera* CameraManager::GetCamera(UUID uuid)
	{
		if (m_IDToCamera.find(uuid) == m_IDToCamera.end()) return nullptr;
		size_t index = m_IDToCamera[uuid];
		return &m_Cameras[index];
	}

	void CameraManager::ResizeAllCameras(glm::uvec2 resolution)
	{
		for (Camera& camera : m_Cameras)
		{
			camera.SetResolution(resolution.x, resolution.y);
		}
	}

	void CameraManager::Cleanup()
	{
		m_Cameras.clear();
		m_UnusedCameraIndices.clear();
		m_IDToCamera.clear();
	}

	CameraManager::CameraManager(Engine* pEngine): m_pEngine(pEngine)
	{
	}

	CameraManager::~CameraManager() {}
}
