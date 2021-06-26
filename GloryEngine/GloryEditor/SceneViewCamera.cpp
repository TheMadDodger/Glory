#include "stdafx.h"
//#include "SceneViewCamera.h"
//#include <InputManager.h>
//#include <TransformComponent.h>
//#include <CameraComponent.h>
//
//namespace Spartan::Editor
//{
//    SceneViewCamera* SceneViewCamera::m_pInstance;
//
//    SceneViewCamera::SceneViewCamera() : BasicCamera()
//    {
//        m_pInstance = this;
//    }
//
//    SceneViewCamera::~SceneViewCamera()
//    {
//    }
//
//    void SceneViewCamera::Initialize(const GameContext& gameContext)
//    {
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-LEFTSHIFT", InputType::Down, -1, NULL, SDL_SCANCODE_LSHIFT));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-RIGHTSHIFT", InputType::Down, -1, NULL, SDL_SCANCODE_RSHIFT));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-LEFT", InputType::Down, 'a', NULL, SDL_SCANCODE_LEFT));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-RIGHT", InputType::Down, 'd', NULL, SDL_SCANCODE_RIGHT));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-FORWARD", InputType::Down, 'w', NULL, SDL_SCANCODE_UP));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-BACKWARD", InputType::Down, 's', NULL, SDL_SCANCODE_DOWN));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-UP", InputType::Down, 'q'));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-DOWN", InputType::Down, 'e'));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-ASCEND", InputType::Down, -1, NULL, SDL_SCANCODE_SPACE));
//        gameContext.pInput->AddInputAction(InputAction("EDITOR-DESCEND", InputType::Down, -1, NULL, SDL_SCANCODE_LCTRL));
//
//        GetCameraComponent()->SetPerspective(60.0f * M_PI / 180.0f, 0.1f, 9999.0f);
//
//        m_LastFrameMouse = gameContext.pInput->GetMouseScreenPosition();
//    }
//
//    void SceneViewCamera::Update(const GameContext& gameContext)
//    {
//        auto fastMode = gameContext.pInput->IsActionTriggered("EDITOR-LEFTSHIFT") || gameContext.pInput->IsActionTriggered("EDITOR-RIGHTSHIFT");
//        auto movementSpeed = fastMode ? m_FastMovementSpeed : m_MovementSpeed;
//
//        float deltaTime = gameContext.pTime->GetDeltaTime() / 1000.0f;
//
//        auto pos = GetTransform()->Position;
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-LEFT"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetLeft() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-RIGHT"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetRight() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-FORWARD"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetBack() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-BACKWARD"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetForward() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-UP"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetUp() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-DOWN"))
//        {
//            GetTransform()->Position = pos + (GetTransform()->Rotation.GetDown() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-ASCEND"))
//        {
//            GetTransform()->Position = pos + (Vector3::Up() * movementSpeed * deltaTime);
//        }
//
//        if (gameContext.pInput->IsActionTriggered("EDITOR-DESCEND"))
//        {
//            GetTransform()->Position = pos + (Vector3::Down() * movementSpeed * deltaTime);
//        }
//
//        auto newMousePos = gameContext.pInput->GetMouseScreenPosition();
//        auto mouseDelta = newMousePos - m_LastFrameMouse;
//        if (m_Looking)
//        {
//            m_Yaw = m_Yaw + mouseDelta.x * m_FreeLookSensitivity;
//            m_Pitch = m_Pitch - mouseDelta.y * m_FreeLookSensitivity;
//        }
//        m_LastFrameMouse = newMousePos;
//
//        GetTransform()->Rotation = Matrix4X4::CreateRotationMatrix(Vector3(m_Pitch, m_Yaw, 0.0f));
//        
//        float axis = gameContext.pInput->GetMouseWheelMovement().y;
//        auto zoomSensitivity = fastMode ? m_FastZoomSensitivity : m_ZoomSensitivity;
//        if (axis > 0) GetTransform()->Position = pos + (GetTransform()->Rotation.GetBack() * movementSpeed * deltaTime);
//        else if (axis < 0) GetTransform()->Position = pos + (GetTransform()->Rotation.GetForward() * movementSpeed * deltaTime);
//        
//        if (gameContext.pInput->IsMouseButtonDown(SDL_BUTTON_RIGHT))
//        {
//            StartLooking();
//        }
//        else
//        {
//            StopLooking();
//        }
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
//
//    SceneViewCamera* SceneViewCamera::GetSceneCamera()
//    {
//        return m_pInstance;
//    }
//
//    void SceneViewCamera::Destroy()
//    {
//        delete m_pInstance;
//    }
//}