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

	bool CameraRef::operator<(const CameraRef& other) const
	{
		return GetPriority() < other.GetPriority();
	}

	void CameraRef::SetResolution(size_t width, size_t height)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetResolution(width, height);
	}

	void CameraRef::SetPerspectiveProjection(size_t width, size_t height, float halfFOV, float near, float far)
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

	void CameraRef::SetPriority(int priority)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetPriority(priority);
	}

	void CameraRef::SetLayerMask(const LayerMask& layerMask)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetLayerMask(layerMask);
	}

	void CameraRef::SetClearColor(const glm::vec4& clearColor)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetClearColor(clearColor);
	}

	void CameraRef::SetOutputTexture(RenderTexture* pTexture)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetOutputTexture(pTexture);
	}

	void CameraRef::EnableOutput(bool enable)
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->EnableOutput(enable);
	}

	const glm::uvec2& CameraRef::GetResolution() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return glm::uvec2();
		return pCamera->GetResolution();
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

	int CameraRef::GetPriority() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return -1;
		return pCamera->GetPriority();
	}

	const glm::vec4& CameraRef::GetClearColor() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return glm::vec4(0.0f);
		return pCamera->GetClearColor();
	}

	const LayerMask& CameraRef::GetLayerMask() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return 0;
		return pCamera->GetLayerMask();
	}

	RenderTexture* CameraRef::GetRenderTexture() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetRenderTexture();
	}

	RenderTexture* CameraRef::GetOutputTexture() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetOutputTexture();
	}

	bool CameraRef::HasOutput() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return false;
		return pCamera->HasOutput();
	}

	void CameraRef::Free()
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		CameraManager::SetUnused(pCamera);
	}

	float CameraRef::GetNear() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return 0.0f;
		return pCamera->GetNear();
	}

	float CameraRef::GetFar() const
	{
		Camera* pCamera = CameraManager::GetCamera(m_CameraID);
		if (pCamera == nullptr) return 0.0f;
		return pCamera->GetFar();
	}
}
