#include "SceneViewCamera.h"
#include <CameraManager.h>
#include <SDL2/SDL.h>

namespace Glory::Editor
{
    SceneViewCamera::SceneViewCamera()
		: m_EyePosition(glm::vec3(2.0f, 2.0f, 2.0f)), m_Rotation(glm::quat_identity<float, glm::defaultp>()) {}

    SceneViewCamera::~SceneViewCamera()
    {
    }

    void SceneViewCamera::Initialize()
    {
        m_Camera = CameraManager::GetNewOrUnusedCamera();
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
        ImGuiIO& io = ImGui::GetIO();
		io.KeyShift;
		
		bool fastMode = io.KeyShift;
		float movementSpeed = fastMode ? m_FastMovementSpeed : m_MovementSpeed;
		
		float deltaTime = io.DeltaTime;
		
		bool leftKey = ImGui::IsKeyDown(SDL_SCANCODE_A);
		bool rightKey = ImGui::IsKeyDown(SDL_SCANCODE_D);
		bool forwardKey = ImGui::IsKeyDown(SDL_SCANCODE_W);
		bool backwardKey = ImGui::IsKeyDown(SDL_SCANCODE_S);
		bool upKey = ImGui::IsKeyDown(SDL_SCANCODE_Q);
		bool downKey = ImGui::IsKeyDown(SDL_SCANCODE_E);
		
		glm::vec3 left = glm::rotate(m_Rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
		glm::vec3 right = glm::rotate(m_Rotation, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 forward = glm::rotate(m_Rotation, glm::vec3(0.0f, 0.0f, -1.0f));
		glm::vec3 backward = glm::rotate(m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 up = glm::vec3(0.0, -1.0f, 0.0f);
		glm::vec3 down = glm::vec3(0.0, 1.0f, 0.0f);
		
		if (leftKey) m_EyePosition = m_EyePosition + left * movementSpeed * deltaTime;
		if (rightKey) m_EyePosition = m_EyePosition + right * movementSpeed * deltaTime;
		if (forwardKey) m_EyePosition = m_EyePosition + forward * movementSpeed * deltaTime;
		if (backwardKey) m_EyePosition = m_EyePosition + backward * movementSpeed * deltaTime;
		if (upKey) m_EyePosition = m_EyePosition + up * movementSpeed * deltaTime;
		if (downKey) m_EyePosition = m_EyePosition + down * movementSpeed * deltaTime;
		
		ImVec2 mouseDelta = io.MouseDelta;
		
		if (m_Looking)
		{
		    m_Yaw = m_Yaw - mouseDelta.x * m_FreeLookSensitivity;
		    m_Pitch = m_Pitch - mouseDelta.y * m_FreeLookSensitivity;
		}

		m_Rotation = glm::quat(glm::vec3(glm::radians(m_Pitch), glm::radians(m_Yaw), 0.0f));

		float axis = io.MouseWheel;
		auto zoomSensitivity = fastMode ? m_FastZoomSensitivity : m_ZoomSensitivity;
		if (axis > 0) m_EyePosition = m_EyePosition + forward * movementSpeed * deltaTime;
		else if (axis < 0) m_EyePosition = m_EyePosition + backward * movementSpeed * deltaTime;
		
		m_Looking = io.MouseDown[1];

		glm::mat4 rotation = glm::toMat4(m_Rotation);
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), m_EyePosition);
		glm::mat4 transform = translation * rotation;

		glm::mat4 view = glm::inverse(transform);
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
}
