#include "CameraRef.h"
#include "CameraManager.h"
#include "RenderTexture.h"
#include "Camera.h"

namespace Glory
{
	CameraRef::CameraRef() : m_pManager(nullptr), m_CameraID(0)
	{
	}

	CameraRef::CameraRef(CameraManager* pManager, UUID uuid) : m_pManager(pManager), m_CameraID(uuid)
	{
	}

	void CameraRef::SetResolution(uint32_t width, uint32_t height)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetResolution(width, height);
	}

	void CameraRef::SetPerspectiveProjection(uint32_t width, uint32_t height, float halfFOV, float near, float far)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetPerspectiveProjection(width, height, halfFOV, near, far);
	}

	void CameraRef::SetOrthographicProjection(float width, float height, float near, float far)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetOrthographicProjection(width, height, near, far);
	}

	void CameraRef::SetView(const glm::mat4& view)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetView(view);
	}

	void CameraRef::SetDisplayIndex(int index)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetDisplayIndex(index);
	}

	void CameraRef::SetPriority(int priority)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetPriority(priority);
	}

	void CameraRef::SetLayerMask(const LayerMask& layerMask)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetLayerMask(layerMask);
	}

	void CameraRef::SetClearColor(const glm::vec4& clearColor)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetClearColor(clearColor);
	}

	void CameraRef::SetOutputTexture(RenderTexture* pTexture)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetOutputTexture(pTexture);
	}

	void CameraRef::EnableOutput(bool enable)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->EnableOutput(enable);
	}

	void CameraRef::SetUserData(const std::string& name, void* data)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		pCamera->SetUserData(name, data);
	}

	const glm::uvec2& CameraRef::GetResolution() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetResolution();
	}

	const glm::mat4& CameraRef::GetView() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetView();
	}

	glm::mat4 CameraRef::GetViewInverse() const
	{
		return glm::inverse(GetView());
	}

	const glm::mat4& CameraRef::GetProjection() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetProjection();
	}

	glm::mat4 CameraRef::GetProjectionInverse() const
	{
		return glm::inverse(GetProjection());
	}

	glm::mat4 CameraRef::GetFinalView() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetFinalView();
	}

	float* CameraRef::GetViewPointer()
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetViewPointer();
	}

	float* CameraRef::GetProjectionPointer()
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetProjectionPointer();
	}

	int CameraRef::GetDisplayIndex() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return -1;
		return pCamera->GetDisplayIndex();
	}

	int CameraRef::GetPriority() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return -1;
		return pCamera->GetPriority();
	}

	const glm::vec4& CameraRef::GetClearColor() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetClearColor();
	}

	const LayerMask& CameraRef::GetLayerMask() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr)
			throw std::exception();
		return pCamera->GetLayerMask();
	}

	RenderTexture* CameraRef::GetRenderTexture() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetRenderTexture();
	}

	RenderTexture* CameraRef::GetOutputTexture() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return nullptr;
		return pCamera->GetOutputTexture();
	}

	bool CameraRef::HasOutput() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return false;
		return pCamera->HasOutput();
	}

	void CameraRef::Free()
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return;
		m_pManager->SetUnused(pCamera);
	}

	void* CameraRef::GetUserDataVoid(const std::string& name)
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return false;
		void* data = nullptr;
		if (!pCamera->GetUserData(name, data)) return nullptr;
		return data;
	}

	float CameraRef::GetNear() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return 0.0f;
		return pCamera->GetNear();
	}

	float CameraRef::GetFar() const
	{
		Camera* pCamera = m_pManager->GetCamera(m_CameraID);
		if (pCamera == nullptr) return 0.0f;
		return pCamera->GetFar();
	}

	UUID CameraRef::GetUUID() const
	{
		return m_CameraID;
	}
}
