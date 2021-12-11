//#include "SceneViewCamera.h"
//#include <CameraManager.h>
//
//namespace Glory::Editor
//{
//    SceneViewCamera::SceneViewCamera() {} // : m_EyePosition(glm::vec3()), m_Rotation(1.0f, 0.0f, 0.0f, 0.0f) {}
//
//    SceneViewCamera::~SceneViewCamera()
//    {
//    }
//
//    void SceneViewCamera::Initialize()
//    {
//        //m_Camera = CameraManager::GetNewOrUnusedCamera();
//    }
//
//    void SceneViewCamera::Update()
//    {
//        //ImGuiIO& io = ImGui::GetIO();
//        //io.KeyShift;
//        //
//        //bool fastMode = io.KeyShift;
//        //float movementSpeed = fastMode ? m_FastMovementSpeed : m_MovementSpeed;
//        //
//        //float deltaTime = io.DeltaTime;
//        //
//        //glm::vec3 pos = m_EyePosition;
//        //
//        //bool leftKey = ImGui::IsKeyDown('A');
//        //bool rightKey = ImGui::IsKeyDown('D');
//        //bool forwardKey = ImGui::IsKeyDown('W');
//        //bool backwardKey = ImGui::IsKeyDown('S');
//        //bool upKey = ImGui::IsKeyDown('Q');
//        //bool downKey = ImGui::IsKeyDown('E');
//        //
//        //glm::vec3 left = glm::rotate(m_Rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
//        //glm::vec3 right = glm::rotate(m_Rotation, glm::vec3(1.0f, 0.0f, 0.0f));
//        //glm::vec3 forward = glm::rotate(m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
//        //glm::vec3 backward = glm::rotate(m_Rotation, glm::vec3(0.0f, 0.0f, -1.0f));
//        //glm::vec3 up = glm::vec3(0.0, 1.0f, 0.0f);
//        //glm::vec3 down = glm::vec3(0.0, -1.0f, 0.0f);
//        //
//        //if (leftKey) m_EyePosition = pos + left * movementSpeed * deltaTime;
//        //if (rightKey) m_EyePosition = pos + right * movementSpeed * deltaTime;
//        //if (forwardKey) m_EyePosition = pos + forward * movementSpeed * deltaTime;
//        //if (backwardKey) m_EyePosition = pos + backward * movementSpeed * deltaTime;
//        //if (upKey) m_EyePosition = pos + up * movementSpeed * deltaTime;
//        //if (downKey) m_EyePosition = pos + down * movementSpeed * deltaTime;
//        //
//        //ImVec2 mouseDelta = io.MouseDelta;
//        //
//        //if (m_Looking)
//        //{
//        //    m_Yaw = m_Yaw + mouseDelta.x * m_FreeLookSensitivity;
//        //    m_Pitch = m_Pitch - mouseDelta.y * m_FreeLookSensitivity;
//        //}
//        //
//        ////GetTransform()->Rotation = Matrix4X4::CreateRotationMatrix(Vector3(m_Pitch, m_Yaw, 0.0f));
//        //float axis = io.MouseWheel;
//        //auto zoomSensitivity = fastMode ? m_FastZoomSensitivity : m_ZoomSensitivity;
//        //if (axis > 0) m_EyePosition = pos + backward * movementSpeed * deltaTime;
//        //else if (axis < 0) m_EyePosition = pos + forward * movementSpeed * deltaTime;
//        //
//        //if (io.MouseDown[1])
//        //{
//        //    StartLooking();
//        //}
//        //else
//        //{
//        //    StopLooking();
//        //}
//        //
//        //glm::mat4 view = glm::translate(glm::identity<glm::mat4>(), m_EyePosition);
//        //m_Camera.SetView(view);
//    }
//
//    void SceneViewCamera::StartLooking()
//    {
//        m_Looking = true;
//    }
//
//    void SceneViewCamera::StopLooking()
//    {
//        m_Looking = false;
//    }
//}
