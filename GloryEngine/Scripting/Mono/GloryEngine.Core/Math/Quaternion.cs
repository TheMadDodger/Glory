using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct Quaternion
    {
        #region Fields

        public float x;
        public float y;
        public float z;
        public float w;

        #endregion

        #region Constructor

        public Quaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Quaternion(Quaternion other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
        }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator +(Quaternion a, Quaternion b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator -(Quaternion a, Quaternion b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator *(Quaternion a, Quaternion b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator /(Quaternion a, Quaternion b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator *(Quaternion a, float factor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Quaternion operator /(Quaternion a, float factor);

        public override string ToString()
        {
            return string.Format("{0},{1},{2},{3}", x, y, z, w);
        }

        #endregion
    }
}
