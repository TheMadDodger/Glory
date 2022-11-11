using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct Vector2
    {
        #region Fields

        public float x;
        public float y;

        #endregion

        #region Constructor

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public Vector2(float xy)
        {
            x = xy;
            y = xy;
        }

        public Vector2(Vector2 other)
        {
            x = other.x;
            y = other.y;
        }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector2 operator +(Vector2 a, Vector2 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector2 operator -(Vector2 a, Vector2 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector2 operator *(Vector2 a, Vector2 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector2 operator /(Vector2 a, Vector2 b);

        public static Vector2 operator *(Vector2 a, float factor)
        {
            return a * new Vector2(factor);
        }

        public static Vector2 operator /(Vector2 a, float factor)
        {
            return a / new Vector2(factor);
        }

        public override string ToString()
        {
            return string.Format("{0},{1}", x, y);
        }

        #endregion
    }
}
