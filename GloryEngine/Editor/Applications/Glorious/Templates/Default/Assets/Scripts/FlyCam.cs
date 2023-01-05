using GloryEngine;
using GloryEngine.Entities;

namespace Sponza
{
    public class FlyCam : EntityBehaviour
    {
        public float _freeLookSensitivity = 1.0f;

        private CameraComponent camera = null;

        public void Start()
        {
            camera = Entity.GetComponent<CameraComponent>();
            Input.SetPlayerInputMode(0, "Keyboard and Mouse");
        }

        public void Update()
        {
            float vertical = Input.GetAxis(0, "Movement", "VerticalAxis");
            float horizontal = Input.GetAxis(0, "Movement", "HorizontalAxis");

            Vector2 mouseDelta;
            mouseDelta.x = Input.GetAxis(0, "Movement", "AimHorizontal");
            mouseDelta.y = Input.GetAxis(0, "Movement", "AimVertical");

            bool use = Input.IsActionTriggered(0, "Interactions", "Use");

            if (use) Debug.LogInfo("Use action triggered!");

            Vector3 pos = Transform.LocalPosition;
            pos = pos - Transform.Forward * vertical + Transform.Right * horizontal;
            Transform.LocalPosition = pos;

            Mat4 rx, ry, roll;

            Vector3 referenceUp = new Vector3(0.0f, 1.0f, 0.0f);
            rx = Mat4.Rotate(-mouseDelta.x * _freeLookSensitivity * Time.DeltaTime, referenceUp);
            ry = Mat4.Rotate(mouseDelta.y * _freeLookSensitivity * Time.DeltaTime, Transform.Right);

            roll = rx * ry;

            Vector4 newDirV4 = new Vector4(Transform.Forward * -1.0f, 1.0f);
            newDirV4 = roll * newDirV4;
            Vector3 newDir = new Vector3(newDirV4);
            newDir = newDir.Normalized;

            Transform.Forward = newDir;
        }

        public void Draw()
        {
        }
    }
}