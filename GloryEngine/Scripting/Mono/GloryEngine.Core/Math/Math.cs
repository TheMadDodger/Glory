using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public static class Math
    {
        #region Methods

        public static Mat4 LookAt(Vector3 eye, Vector3 center, Vector3 up)
            => Math_LookAt(eye, center, up);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Math_LookAt(Vector3 eye, Vector3 center, Vector3 up);

        #endregion
    }
}
