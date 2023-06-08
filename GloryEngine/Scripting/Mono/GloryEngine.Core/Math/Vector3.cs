namespace GloryEngine
{
    public struct Vector3
    {
        #region Props

        public Vector3 Normalized => new Vector3(x, y, z) / Length;

        public float Length => (float)System.Math.Sqrt(x*x + y*y + z*z);

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

        public float Dot(Vector3 other)
        {
            return Dot(this, other);
        }

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

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        public override string ToString()
        {
            return string.Format("{0},{1},{2}", x, y, z);
        }

        #endregion
    }
}
