namespace GloryEngine
{
    public struct Vector4
    {
        #region Props

        /// <summary>
        /// Get normalized vector
        /// </summary>
        public Vector4 Normalized => new Vector4(x, y, z, w) / Length;

        /// <summary>
        /// Length of the vector
        /// </summary>
        public float Length => (float)System.Math.Sqrt(x * x + y * y + z * z + w * w);

        #endregion

        #region Fields

        /// <summary>
        /// X component
        /// </summary>
        public float x;

        /// <summary>
        /// Y component
        /// </summary>
        public float y;

        /// <summary>
        /// Z component
        /// </summary>
        public float z;

        /// <summary>
        /// W component
        /// </summary>
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

        /// <summary>
        /// Calculate the Dot product between this vector and another
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public float Dot(Vector4 other)
        {
            return Dot(this, other);
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public Vector4 Lerp(Vector4 other, float t)
        {
            return Lerp(this, other, t);
        }

        /// <summary>
        /// Calculate the dot product between 2 vectors
        /// </summary>
        /// <param name="a">Vector a</param>
        /// <param name="b">Vector b</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="a">Starting vector</param>
        /// <param name="b">Target vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            Vector4 result = new Vector4();
            result.x = Math.Lerp(a.x, b.x, t);
            result.y = Math.Lerp(a.y, b.y, t);
            result.z = Math.Lerp(a.z, b.z, t);
            result.w = Math.Lerp(a.w, b.w, t);
            return result;
        }

        public override string ToString()
        {
            return string.Format("{0},{1},{2},{3}", x, y, z, w);
        }

        #endregion

        #region Operator Overloads

        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Vector4 operator -(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Vector4 operator *(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        public static Vector4 operator /(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
        }

        public static Vector4 operator *(Vector4 a, float factor)
        {
            return a * new Vector4(factor);
        }

        public static Vector4 operator /(Vector4 a, float factor)
        {
            return a / new Vector4(factor);
        }

        #endregion
    }
}
