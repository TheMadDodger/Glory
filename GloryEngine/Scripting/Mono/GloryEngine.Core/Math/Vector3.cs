namespace GloryEngine
{
    public struct Vector3
    {
        #region Props

        /// <summary>
        /// Get normalized vector
        /// </summary>
        public Vector3 Normalized => new Vector3(x, y, z) / Length;

        /// <summary>
        /// Length of the vector
        /// </summary>
        public float Length => (float)System.Math.Sqrt(x*x + y*y + z*z);

        /// <summary>
        /// Get a Vector2 with the x and y component of this Vector3
        /// </summary>
        public Vector2 xy => new Vector2(x, y);

        /// <summary>
        /// Get a Vector2 with the y and z component of this Vector3
        /// </summary>
        public Vector2 yz => new Vector2(y, z);

        /// <summary>
        /// Get a Vector2 with the x and z component of this Vector3
        /// </summary>
        public Vector2 xz => new Vector2(x, z);

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

        public Vector3(Vector2 other)
        {
            x = other.x;
            y = other.y;
            z = 0.0f;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Calculate the Dot product between this vector and another
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public float Dot(Vector3 other)
        {
            return Dot(this, other);
        }

        /// <summary>
        /// Calculate the cross product of this vector and another
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <returns>The cross product of the 2 vectors</returns>
        public Vector3 Cross(Vector3 other)
        {
            return Cross(this, other);
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public Vector3 Lerp(Vector3 other, float t)
        {
            return Lerp(this, other, t);
        }

        /// <summary>
        /// Calculate the dot product between 2 vectors
        /// </summary>
        /// <param name="a">Vector a</param>
        /// <param name="b">Vector b</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        /// <summary>
        /// Calculate the cross product of 2 vectors
        /// </summary>
        /// <param name="a">Vector a</param>
        /// <param name="b">Vector b</param>
        /// <returns>The cross product of the 2 vectors</returns>
        public static Vector3 Cross(Vector3 a, Vector3 b)
        {
            float cx = a.y*b.z - a.z*b.y;
            float cy = a.z*b.x - a.x*b.z;
            float cz = a.x*b.y - a.y*b.x;
            return new Vector3(cx, cy, cz);
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="a">Starting vector</param>
        /// <param name="b">Target vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            Vector3 result = new Vector3();
            result.x = Math.Lerp(a.x, b.x, t);
            result.y = Math.Lerp(a.y, b.y, t);
            result.z = Math.Lerp(a.z, b.z, t);
            return result;
        }

        public override string ToString()
        {
            return string.Format("{0},{1},{2}", x, y, z);
        }

        #endregion

        #region Operator Overloads

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
        }

        public static Vector3 operator /(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
        }

        public static Vector3 operator *(Vector3 a, float factor)
        {
            return a * new Vector3(factor);
        }

        public static Vector3 operator /(Vector3 a, float factor)
        {
            return a / new Vector3(factor);
        }

        #endregion
    }
}
