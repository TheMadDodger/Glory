#include "CameraRef.h"
#include "CameraManager.h"

namespace Glory
{
	CameraRef::CameraRef() : m_CameraID()
	{
	}

	CameraRef::CameraRef(UUID uuid) : m_CameraID(uuid)
	{
	}

	void CameraRef::SetResolution(int width, int height)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetResolution(width, height);
	}

	void CameraRef::SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetPerspectiveProjection(width, height, halfFOV, near, far);
	}

	void CameraRef::SetView(const glm::mat4& view)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetView(view);
	}

	void CameraRef::SetDisplayIndex(int index)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetDisplayIndex(index);
	}

	const glm::mat4& CameraRef::GetView() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return glm::mat4();
		return pCamera->GetView();
	}

	const glm::mat4& CameraRef::GetProjection() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return glm::mat4();
		return pCamera->GetProjection();
	}

	int CameraRef::GetDisplayIndex() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return -1;
		return pCamera->GetDisplayIndex();
	}

	RenderTexture* CameraRef::GetRenderTexture() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetRenderTexture();
	}

	void CameraRef::Free()
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		CameraManager::SetUnused(pCamera);
	}
}
