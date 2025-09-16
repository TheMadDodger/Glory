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

	Camera* CameraManager::GetCamera(UUID uuid)
	{
		if (m_IDToCamera.find(uuid) == m_IDToCamera.end()) return nullptr;
		size_t index = m_IDToCamera[uuid];
		return &m_Cameras[index];
	}

	void CameraManager::OnCameraResized(CameraRef camera)
	{
		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		pRenderer->OnCameraResize(camera);
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
