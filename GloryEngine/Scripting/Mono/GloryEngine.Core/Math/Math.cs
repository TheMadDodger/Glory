using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public static class Math
    {
        #region Methods

        /// <summary>
        /// Generate a matrix that looks at a certain object from a point
        /// </summary>
        /// <param name="eye">Position of the eye</param>
        /// <param name="center">Position to look at</param>
        /// <param name="up">Up vector</param>
        /// <returns></returns>
        public static Mat4 LookAt(Vector3 eye, Vector3 center, Vector3 up)
            => Math_LookAt(eye, center, up);

        /// <summary>
        /// Linear interpolate between 2 floats
        /// </summary>
        /// <param name="a">Starting value</param>
        /// <param name="b">Target value</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped value</returns>
        public static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mat4 Math_LookAt(Vector3 eye, Vector3 center, Vector3 up);

        #endregion
    }
}
