#pragma once
#include <CameraRef.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Glory::Editor
{
    class SceneViewCamera
    {
    public:
        SceneViewCamera();
        virtual ~SceneViewCamera();

        void StartLooking();
        void StopLooking();

        void SetPerspective(uint32_t width, uint32_t height, float halfFOV, float near, float far);
        void SetOrthographic(uint32_t width, uint32_t height, float near, float far);

        //float GetOrthoZoom();

    private:
        void Initialize();
        void Cleanup();
        void Update();

    private:
        friend class SceneWindow;
        /// Normal speed of camera movement.
        float m_MovementSpeed = 10.f;

        /// Speed of camera movement when shift is held down,
        float m_FastMovementSpeed = 100.f;

        /// Sensitivity for free look.
        float m_FreeLookSensitivity = 1.f;

        /// Amount to zoom the camera when using the mouse wheel.
        float m_ZoomSensitivity = 10.f;

        /// Amount to zoom the camera when using the mouse wheel (fast mode).
        float m_FastZoomSensitivity = 50.f;

        bool m_IsOrthographic = false;
        float m_HalfFOV = 60.0f;
        uint32_t m_Width = 1;
        uint32_t m_Height = 1;
        float m_Near = 0.1f;
        float m_Far = 3000.0f;
        float m_OrthoZoom = 0.1f;

        /// Set to true when free looking (on right mouse button).
        bool m_Looking = false;

        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;

        CameraRef m_Camera;
    };
}