using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct Vector4
    {
        #region Props

        public Vector4 Normalized => Vector4_GetNormalized(this);

        #endregion

        #region Fields

        public float x;
        public float y;
        public float z;
        public float w;

        #endregion

        #region Constructor

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Vector4(float xyzw)
        {
            x = xyzw;
            y = xyzw;
            z = xyzw;
            w = xyzw;
        }

        public Vector4(Vector4 other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
        }

        public Vector4(Vector3 vec3, float w)
        {
            x = vec3.x;
            y = vec3.y;
            z = vec3.z;
            this.w = w;
        }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 Vector4_GetNormalized(Vector4 a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 operator +(Vector4 a, Vector4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 operator -(Vector4 a, Vector4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 operator *(Vector4 a, Vector4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 operator /(Vector4 a, Vector4 b);

        public static Vector4 operator *(Vector4 a, float factor)
        {
            return a * new Vector4(factor);
        }

        public static Vector4 operator /(Vector4 a, float factor)
        {
            return a / new Vector4(factor);
        }

        public override string ToString()
        {
            return string.Format("{0},{1},{2},{3}", x, y, z, w);
        }

        #endregion
    }
}
