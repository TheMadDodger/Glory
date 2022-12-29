using System.Runtime.CompilerServices;
using System.Text;

namespace GloryEngine
{
    public struct Mat4
    {
        #region Props

        public static Mat4 Identity => Mat4_GetIdentity();

        public Mat4 Inverse => Mat4_GetInverse(this);

        #endregion

        #region Fields

        public Vector4 Vec1;
        public Vector4 Vec2;
        public Vector4 Vec3;
        public Vector4 Vec4;

        #endregion

        #region Methods

        public static Mat4 Translate(Vector3 translation) => Mat4_Translate(translation);
        public static Mat4 Translate(Mat4 m, Vector3 translation) => Mat4_TranslateMat4(m, translation);
        public static Mat4 Rotate(float angle, Vector3 axis) => Mat4_Rotate(angle, axis);
        public static Mat4 Rotate(Mat4 m, float angle, Vector3 axis) => Mat4_RotateMat4(m, angle, axis);
        public static Mat4 Scale(Vector3 scale) => Mat4_Scale(scale);
        public static Mat4 Scale(Mat4 m, Vector3 scale) => Mat4_ScaleMat4(m, scale);

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("V1: ").Append(Vec1).Append(", ");
            sb.Append("V2: ").Append(Vec2).Append(", ");
            sb.Append("V3: ").Append(Vec3).Append(", ");
            sb.Append("V4: ").Append(Vec4);
            return sb.ToString();
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_GetIdentity();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_GetInverse(Mat4 m);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_Translate(Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_TranslateMat4(Mat4 m, Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_Rotate(float angle, Vector3 axis);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_RotateMat4(Mat4 m, float angle, Vector3 axis);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_Scale(Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Mat4_ScaleMat4(Mat4 m, Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Mat4 operator +(Mat4 a, Mat4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Mat4 operator -(Mat4 a, Mat4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Mat4 operator *(Mat4 a, Mat4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Mat4 operator /(Mat4 a, Mat4 b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector4 operator *(Mat4 a, Vector4 b);

        #endregion
    }
}
