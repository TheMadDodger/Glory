//#pragma once
//#include <BasicCamera.h>
//
//namespace Spartan::Editor
//{
//    class SceneViewCamera : public Spartan::BasicCamera
//    {
//    public:
//        SceneViewCamera();
//        virtual ~SceneViewCamera();
//
//        void StartLooking();
//        void StopLooking();
//
//        static SceneViewCamera* GetSceneCamera();
//        static void Destroy();
//
//    private:
//        virtual void Initialize(const GameContext& gameContext) override;
//        virtual void Update(const GameContext& gameContext) override;
//
//    private:
//        /// Normal speed of camera movement.
//        float m_MovementSpeed = 10.f;
//
//        /// Speed of camera movement when shift is held down,
//        float m_FastMovementSpeed = 100.f;
//
//        /// Sensitivity for free look.
//        float m_FreeLookSensitivity = 1.f;
//
//        /// Amount to zoom the camera when using the mouse wheel.
//        float m_ZoomSensitivity = 10.f;
//
//        /// Amount to zoom the camera when using the mouse wheel (fast mode).
//        float m_FastZoomSensitivity = 50.f;
//
//        /// Set to true when free looking (on right mouse button).
//        bool m_Looking = false;
//
//        float m_Yaw = 0.0f;
//        float m_Pitch = 0.0f;
//
//        Vector2 m_LastFrameMouse;
//
//        static SceneViewCamera* m_pInstance;
//    };
//}