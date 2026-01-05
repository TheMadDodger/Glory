#include "CameraManager.h"
#include "Engine.h"
#include "RendererModule.h"
#include "WindowModule.h"
#include "Window.h"
#include "EngineProfiler.h"
#include "Camera.h"

namespace Glory
{
	CameraRef CameraManager::GetNewOrUnusedCamera()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "CameraManager::GetNewOrUnusedCamera" };
		int width, height;
		m_pEngine->GetMainModule<WindowModule>()->GetMainWindow()->GetDrawableSize(&width, &height);

		if (m_UnusedCameraIndices.size() > 0)
		{
			size_t index = m_UnusedCameraIndices[0];
			m_UnusedCameraIndices.erase(m_UnusedCameraIndices.begin());
			Camera& pCamera = m_Cameras[index];
			pCamera.SetBaseResolution(float(width), float(height));
			pCamera.m_IsInUse = true;
			return CameraRef(this, pCamera.GetUUID());
		}

		size_t index = m_Cameras.size();
		m_Cameras.push_back(Camera(uint32_t(width), uint32_t(height)));
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
