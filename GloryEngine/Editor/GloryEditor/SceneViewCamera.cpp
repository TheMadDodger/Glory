#include "SceneViewCamera.h"

#include <CameraManager.h>
#include <ImGuizmo.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <Shortcuts.h>

namespace Glory::Editor
{
    SceneViewCamera::SceneViewCamera() {}

    SceneViewCamera::~SceneViewCamera()
    {
    }

    void SceneViewCamera::Initialize()
    {
        m_Camera = GloryContext::GetCameraManager()->GetNewOrUnusedCamera();
		m_Camera.SetDisplayIndex(-1);
		m_Camera.SetPriority(-69420);
		m_Camera.SetClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

	void SceneViewCamera::Cleanup()
	{
		m_Camera.Free();
	}

    void SceneViewCamera::Update()
    {
		glm::mat4 view = m_Camera.GetView();
		const glm::mat4 viewInverse = m_Camera.GetViewInverse();

		glm::vec3 position(viewInverse[3][0], viewInverse[3][1], viewInverse[3][2]);

        const ImGuiIO& io = ImGui::GetIO();

		const bool fastMode = io.KeyShift;
		const float movementSpeed = fastMode ? m_MovementSpeed * 5.0f : m_MovementSpeed;

		const float deltaTime = io.DeltaTime;

		const bool leftKey = ImGui::IsKeyDown(ImGuiKey_A);
		const bool rightKey = ImGui::IsKeyDown(ImGuiKey_D);
		const bool forwardKey = ImGui::IsKeyDown(ImGuiKey_W);
		const bool backwardKey = ImGui::IsKeyDown(ImGuiKey_S);
		const bool upKey = ImGui::IsKeyDown(ImGuiKey_Q);
		const bool downKey = ImGui::IsKeyDown(ImGuiKey_E);

		const glm::vec3 right(viewInverse[0][0], viewInverse[0][1], viewInverse[0][2]);
		const glm::vec3 left = -right;
		const glm::vec3 forward(viewInverse[2][0], viewInverse[2][1], viewInverse[2][2]);
		const glm::vec3 backward = -forward;
		const glm::vec3 referenceUp = glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 referenceDown = glm::vec3(0.0f, -1.0f, 0.0f);

		if (leftKey) position += left * movementSpeed * deltaTime;
		if (rightKey) position += right * movementSpeed * deltaTime;
		if (forwardKey) position -= forward * movementSpeed * deltaTime;
		if (backwardKey) position -= backward * movementSpeed * deltaTime;
		if (upKey) position += referenceDown * movementSpeed * deltaTime;
		if (downKey) position += referenceUp * movementSpeed * deltaTime;

		const float axis = io.MouseWheel;
		const auto zoomSensitivity = fastMode ? m_FastZoomSensitivity : m_ZoomSensitivity;
		if (axis > 0)
		{
			if (!m_IsOrthographic) position += forward * movementSpeed * deltaTime;
			else m_OrthoZoom -= movementSpeed * deltaTime;
		}
		else if (axis < 0)
		{
			if (!m_IsOrthographic) position += backward * movementSpeed * deltaTime;
			else m_OrthoZoom += movementSpeed * deltaTime;
		}

		const ImVec2 mouseDelta = io.MouseDelta;

		glm::mat4 transform = viewInverse;
		view = glm::inverse(transform);

		m_Looking = io.MouseDown[1];
		if (m_Looking)
		{
			/* Block camera keys from triggering shortcuts */
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_W);
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_A);
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_S);
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_D);
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_Q);
			Shortcuts::AddBlockKeyForOneFrame(ImGuiKey_E);

			glm::mat4 rx, ry, roll;

			rx = glm::rotate(glm::identity<glm::mat4>(), -mouseDelta.x * m_FreeLookSensitivity * deltaTime, referenceUp);
			ry = glm::rotate(glm::identity<glm::mat4>(), -mouseDelta.y * m_FreeLookSensitivity * deltaTime, right);

			roll = rx * ry;

			glm::vec4 newDirV4 = glm::vec4(forward, 1.0f);
			newDirV4 = roll * newDirV4;
			glm::vec3 newDir = glm::vec3(newDirV4);
			newDir = glm::normalize(newDir);

			transform = glm::lookAt(position, position - newDir, referenceUp);
			view = transform;
		}

		m_Camera.SetView(view);
    }

    void SceneViewCamera::StartLooking()
    {
        m_Looking = true;
    }

    void SceneViewCamera::StopLooking()
    {
        m_Looking = false;
    }

	bool SceneViewCamera::SetResolution(uint32_t width, uint32_t height)
	{
		const bool diff = width != m_Width || height != m_Height;
		m_Width = width;
		m_Height = height;
		m_Camera.SetResolution(m_Width, m_Height);
		return diff;
	}

	void SceneViewCamera::SetPerspective(uint32_t width, uint32_t height, float halfFOV, float near, float far)
	{
		if (!m_IsOrthographic && width == m_Width && height == m_Height && halfFOV == m_HalfFOV && near == m_Near && far == m_Far)
			return;

		m_HalfFOV = halfFOV;
		m_Width = width;
		m_Height = height;
		m_Near = near;
		m_Far = far;

		m_IsOrthographic = false;
		m_Camera.SetPerspectiveProjection(m_Width, m_Height, m_HalfFOV, m_Near, m_Far);
	}
	void SceneViewCamera::SetOrthographic(uint32_t width, uint32_t height, float near, float far)
	{
		if (m_IsOrthographic && width == m_Width && height == m_Height && near == m_Near && far == m_Far)
			return;

		m_Width = width;
		m_Height = height;
		m_Near = near;
		m_Far = far;

		m_IsOrthographic = true;
		m_Camera.SetOrthographicProjection(m_Width * m_OrthoZoom, m_Height * m_OrthoZoom, m_Near, m_Far);
	}

	void SceneViewCamera::UpdateCamera()
	{
		if (m_IsOrthographic)
			m_Camera.SetOrthographicProjection(m_Width * m_OrthoZoom, m_Height * m_OrthoZoom, m_Near, m_Far);
		else
			m_Camera.SetPerspectiveProjection(m_Width, m_Height, m_HalfFOV, m_Near, m_Far);
	}
}
