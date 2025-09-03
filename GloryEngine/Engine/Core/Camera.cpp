#include "Camera.h"
#include <glm/ext/matrix_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glory
{
	Camera::Camera(uint32_t width, uint32_t height)
		: m_DisplayIndex(0), m_Resolution(width, height), m_TextureIsDirty(true), m_IsOrtho(false),
		m_IsInUse(true), m_View(1.0f), m_Projection(1.0f), m_pRenderTextures(),
		m_ClearColor(glm::vec4(0.0f)), m_Priority(0), m_LayerMask(0), m_Near(0.0f), m_Far(0.0f), m_HalfFOV(60.0f),
		m_pOutputTexture(nullptr), m_pSecondaryOutputTexture(nullptr),
		m_PerspectiveDirty(true), m_ViewOffset(glm::identity<glm::mat4>())
	{

	}

	void Camera::SetResolution(uint32_t width, uint32_t height)
	{
		if (m_Resolution.x == width && m_Resolution.y == height) return;
		m_Resolution = glm::ivec2(width, height);
		m_TextureIsDirty = true;

		if (!m_IsOrtho)
			SetPerspectiveProjection(width, height, m_HalfFOV, m_Near, m_Far);
		else
			SetOrthographicProjection(width, height, m_Near, m_Far);
	}

	void Camera::SetPerspectiveProjection(uint32_t width, uint32_t height, float halfFOV, float near, float far)
	{
		if (m_HalfFOV == halfFOV && m_Near == near && m_Far == far) return;

		m_HalfFOV = halfFOV;
		m_Near = near;
		m_Far = far;

		m_Projection = glm::perspective(glm::radians(halfFOV), (float)width / (float)height, near, far);
		m_ViewOffset = glm::identity<glm::mat4>();

		m_IsOrtho = false;
		m_PerspectiveDirty = true;
	}

	void Camera::SetOrthographicProjection(float width, float height, float near, float far)
	{
		if (m_Near == near && m_Far == far) return;

		m_Near = near;
		m_Far = far;

		m_Projection = glm::ortho<float>(0.0f, width, 0.0f, height, near, far);
		m_ViewOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(width / 2.0f, height / 2.0f, 0.0f));

		m_IsOrtho = false;
		m_PerspectiveDirty = true;
	}

	void Camera::SetView(const glm::mat4& view)
	{
		m_View = view;
	}

	void Camera::SetDisplayIndex(int index)
	{
		m_DisplayIndex = index;
	}

	void Camera::SetPriority(int priority)
	{
		m_Priority = priority;
	}

	void Camera::SetLayerMask(const LayerMask& layerMask)
	{
		m_LayerMask = layerMask;
	}

	void Camera::SetClearColor(const glm::vec4& clearColor)
	{
		m_ClearColor = clearColor;
	}

	void Camera::SetOutputTexture(RenderTexture* pTexture)
	{
		m_pOutputTexture = pTexture;
	}

	void Camera::SetSecondaryOutputTexture(RenderTexture* pTexture)
	{
		m_pSecondaryOutputTexture = pTexture;
	}

	void Camera::Swap()
	{
		RenderTexture* pTempTexture = m_pOutputTexture;
		m_pOutputTexture = m_pSecondaryOutputTexture;
		m_pSecondaryOutputTexture = pTempTexture;
	}

	void Camera::SetUserData(const std::string& name, void* data)
	{
		m_UserDatas[name] = data;
	}

	const glm::uvec2& Camera::GetResolution() const
	{
		return m_Resolution;
	}

	const glm::mat4& Camera::GetView() const
	{
		return m_View;
	}

	const glm::mat4& Camera::GetViewOffset() const
	{
		return m_ViewOffset;
	}

	const glm::mat4& Camera::GetProjection() const
	{
		return m_Projection;
	}

	glm::mat4 Camera::GetFinalView() const
	{
		return m_ViewOffset * m_View;
	}

	float* Camera::GetViewPointer()
	{
		return &m_View[0][0];
	}

	float* Camera::GetProjectionPointer()
	{
		return &m_Projection[0][0];
	}

	int Camera::GetDisplayIndex() const
	{
		return m_DisplayIndex;
	}

	int Camera::GetPriority() const
	{
		return m_Priority;
	}

	const glm::vec4& Camera::GetClearColor() const
	{
		return m_ClearColor;
	}

	const LayerMask& Camera::GetLayerMask() const
	{
		return m_LayerMask;
	}

	size_t Camera::RenderTextureCount() const
	{
		return m_pRenderTextures.size();
	}

	RenderTexture* Camera::GetRenderTexture(size_t index) const
	{
		return m_pRenderTextures[index];
	}

	RenderTexture* Camera::GetOutputTexture() const
	{
		return m_pOutputTexture;
	}

	RenderTexture* Camera::GetSecondaryOutputTexture() const
	{
		return m_pSecondaryOutputTexture;
	}

	uint64_t& Camera::GetUserHandle(const std::string& name)
	{
		return m_UserHandles[name];
	}

	float Camera::GetNear() const
	{
		return m_Near;
	}

	float Camera::GetFar() const
	{
		return m_Far;
	}

	UUID Camera::GetUUID() const
	{
		return m_UUID;
	}
}
