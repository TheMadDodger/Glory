using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct Vector3
    {
        #region Props

        public Vector3 Normalized => Vector3_GetNormalized(this);

        #endregion

        #region Fields

        public float x;
        public float y;
        public float z;

        #endregion

        #region Constructor

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(float xyz)
        {
            x = xyz;
            y = xyz;
            z = xyz;
        }

        public Vector3(Vector3 other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        public Vector3(Vector4 other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3 Vector3_GetNormalized(Vector3 a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3 operator +(Vector3 a, Vector3 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3 operator -(Vector3 a, Vector3 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3 operator *(Vector3 a, Vector3 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3 operator /(Vector3 a, Vector3 b);

        public static Vector3 operator *(Vector3 a, float factor)
        {
            return a * new Vector3(factor);
        }

        public static Vector3 operator /(Vector3 a, float factor)
        {
            return a / new Vector3(factor);
        }

        public override string ToString()
        {
            return string.Format("{0},{1},{2}", x, y, z);
        }

        #endregion
    }
}
