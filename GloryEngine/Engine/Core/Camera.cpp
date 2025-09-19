#include "Camera.h"

#include <glm/ext/matrix_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glory
{
	Camera::Camera(uint32_t width, uint32_t height)
		: m_IsInUse(true), m_IsOrtho(false), m_Output(false), m_PerspectiveDirty(true),
		m_ResolutionDirty(true), m_View(1.0f), m_Projection(1.0f), m_BaseResolution(width, height),
		m_Resolution(width, height), m_ResolutionScale(1.0f, 1.0f), m_ClearColor(glm::vec4(0.0f)),
		m_OutputOffset(), m_Priority(0), m_LayerMask(0), m_Near(0.0f),
		m_Far(0.0f), m_HalfFOV(60.0f), m_ViewOffset(glm::identity<glm::mat4>())
	{}

	bool Camera::SetBaseResolution(uint32_t width, uint32_t height)
	{
		if (m_BaseResolution.x == width && m_BaseResolution.y == height) return false;
		m_BaseResolution = glm::ivec2(width, height);
		m_Resolution = m_ResolutionScale*glm::vec2(m_BaseResolution);

		if (!m_IsOrtho)
			SetPerspectiveProjection(m_HalfFOV, m_Near, m_Far, true);
		else
			SetOrthographicProjection(m_Near, m_Far, true);

		m_ResolutionDirty = true;
		return true;
	}

	const glm::uvec2& Camera::GetBaseResolution() const
	{
		return m_BaseResolution;
	}

	bool Camera::SetResolutionScale(float width, float height)
	{
		if (width == m_ResolutionScale.x && height == m_ResolutionScale.y) return false;
		m_ResolutionScale = { width, height };
		m_Resolution = m_ResolutionScale*glm::vec2(m_BaseResolution);
		m_ResolutionDirty = true;

		if (!m_IsOrtho)
			SetPerspectiveProjection(m_HalfFOV, m_Near, m_Far, true);
		else
			SetOrthographicProjection(m_Near, m_Far, true);

		return true;
	}

	void Camera::SetPerspectiveProjection(float halfFOV, float near, float far, bool force)
	{
		if (!force && m_HalfFOV == halfFOV && m_Near == near && m_Far == far) return;

		m_HalfFOV = halfFOV;
		m_Near = near;
		m_Far = far;

		m_Projection = glm::perspective(glm::radians(halfFOV), (float)m_Resolution.x/(float)m_Resolution.y, near, far);
		m_ViewOffset = glm::identity<glm::mat4>();

		m_IsOrtho = false;
		m_PerspectiveDirty = true;
	}

	void Camera::SetOutput(bool output, int x, int y)
	{
		m_Output = output;
		m_OutputOffset = { x,y };
	}

	bool Camera::IsOutput() const
	{
		return m_Output;
	}

	void Camera::SetOrthographicProjection(float near, float far, bool force)
	{
		if (!force && m_Near == near && m_Far == far) return;

		m_Near = near;
		m_Far = far;

		m_Projection = glm::ortho<float>(0.0f, float(m_Resolution.x), 0.0f, float(m_Resolution.y), near, far);
		m_ViewOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(m_Resolution.x/2.0f, m_Resolution.y/2.0f, 0.0f));

		m_IsOrtho = false;
		m_PerspectiveDirty = true;
	}

	void Camera::SetView(const glm::mat4& view)
	{
		m_View = view;
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

	void Camera::SetUserData(const std::string& name, void* data)
	{
		m_UserDatas[name] = data;
	}

	bool Camera::IsResolutionDirty()
	{
		return m_ResolutionDirty;
	}

	bool Camera::IsPerspectiveDirty()
	{
		return m_PerspectiveDirty;
	}

	void Camera::SetResolutionDirty(bool dirty)
	{
		m_ResolutionDirty = dirty;
	}

	void Camera::SetPerspectiveDirty(bool dirty)
	{
		m_PerspectiveDirty = dirty;
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
		return m_ViewOffset*m_View;
	}

	float* Camera::GetViewPointer()
	{
		return &m_View[0][0];
	}

	float* Camera::GetProjectionPointer()
	{
		return &m_Projection[0][0];
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
