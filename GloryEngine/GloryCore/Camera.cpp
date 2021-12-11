#include "Camera.h"
#include <glm/ext/matrix_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glory
{
	Camera::Camera(int width, int height) : m_Resolution(width, height), m_TextureIsDirty(true), m_IsInUse(true), m_View(1.0f), m_Projection(1.0f), m_pRenderTexture(nullptr)
	{

	}

	void Camera::SetResolution(int width, int height)
	{
		if (m_Resolution.x == width && m_Resolution.y == height) return;
		m_Resolution = glm::ivec2(width, height);
		m_TextureIsDirty = true;
	}

	void Camera::SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far)
	{
		m_Projection = glm::perspective(glm::radians(halfFOV), (float)width / (float)height, near, far);
	}

	void Camera::SetView(const glm::mat4& view)
	{
		m_View = view;
	}

	const glm::mat4& Camera::GetView() const
	{
		return m_View;
	}

	const glm::mat4& Camera::GetProjection() const
	{
		return m_Projection;
	}

	RenderTexture* Camera::GetRenderTexture()
	{
		return m_pRenderTexture;
	}
}
