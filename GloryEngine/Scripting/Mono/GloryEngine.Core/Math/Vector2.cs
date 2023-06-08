namespace GloryEngine
{
    public struct Vector2
    {
        #region Props

        /// <summary>
        /// Get normalized vector
        /// </summary>
        public Vector2 Normalized => new Vector2(x, y) / Length;

        /// <summary>
        /// Length of the vector
        /// </summary>
        public float Length => (float)System.Math.Sqrt(x * x + y * y);

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

        /// <summary>
        /// Calculate the Dot product between this vector and another
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public float Dot(Vector2 other)
        {
            return Dot(this, other);
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="other">The other vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public Vector2 Lerp(Vector2 other, float t)
        {
            return Lerp(this, other, t);
        }

        /// <summary>
        /// Calculate the dot product between 2 vectors
        /// </summary>
        /// <param name="a">Vector a</param>
        /// <param name="b">Vector b</param>
        /// <returns>The dot product of the 2 vectors</returns>
        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.x * b.x + a.y * b.y;
        }

        /// <summary>
        /// Linear interpolate between this vector and another by factor t
        /// </summary>
        /// <param name="a">Starting vector</param>
        /// <param name="b">Target vector</param>
        /// <param name="t">Lerp factor</param>
        /// <returns>The lerped vector</returns>
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
        {
            Vector2 result = new Vector2();
            result.x = Math.Lerp(a.x, b.x, t);
            result.y = Math.Lerp(a.y, b.y, t);
            return result;
        }

        public override string ToString()
        {
            return string.Format("{0},{1}", x, y);
        }

        #endregion

        #region Operator Overloads

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x - b.x, a.y - b.y);
        }

        public static Vector2 operator *(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x * b.x, a.y * b.y);
        }

        public static Vector2 operator /(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x / b.x, a.y / b.y);
        }

        public static Vector2 operator *(Vector2 a, float factor)
        {
            return a * new Vector2(factor);
        }

        public static Vector2 operator /(Vector2 a, float factor)
        {
            return a / new Vector2(factor);
        }

        #endregion
    }
}
