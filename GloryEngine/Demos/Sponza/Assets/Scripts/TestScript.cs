using System;
using GloryEngine;
using GloryEngine.Entities;

namespace Sponza
{
    public class Test : EntityBehaviour
    {
        public float _angularSpeed = 10;
        public int _anInt = 10;
        public double _aDouble = 0.0;
        public bool _aBool = false;
        public float _time = 0;

        public void Update()
        {
            Transform transform = Transform;
            Vector3 position = transform.LocalPosition;
            _time += Time.DeltaTime;
            position.x = _angularSpeed * _time;
            transform.LocalPosition = position;
        }

        public void Draw()
        {
        }
    }
}