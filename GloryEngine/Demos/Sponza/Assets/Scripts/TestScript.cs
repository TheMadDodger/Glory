using System;
using GloryEngine;
using GloryEngine.Entities;

namespace Sponza
{
    public class Test : EntityBehaviour
    {
        float _angularSpeed = 10;
        float _time = 0;

        public void Update()
        {
            Transform transform = Transform;
            Vector3 rotation = transform.LocalRotationEuler;
            _time += Time.DeltaTime;
            rotation.y = _angularSpeed * _time;
            transform.LocalRotationEuler = rotation;
        }

        public void Draw()
        {
        }
    }
}