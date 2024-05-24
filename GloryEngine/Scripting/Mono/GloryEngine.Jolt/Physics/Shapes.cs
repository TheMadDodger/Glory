using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Shape
    /// </summary>
    public struct Shape
    {
        /// <summary>
        /// Internal ID of the shape
        /// </summary>
        public UInt64 ShapeID => _shapeID;

        public Shape(UInt64 id)
        {
            _shapeID = id;
        }

        private UInt64 _shapeID;
    }

    /// <summary>
    /// Shape manager
    /// </summary>
    public static class Shapes
    {
        #region Methods

        /// <summary>
        /// Create a box shape and retreive its handle
        /// </summary>
        /// <param name="extends">Extends of the box</param>
        /// <returns>Shape handle</returns>
        public static Shape Box(Vector3 extends)
        {
            return new Shape(Shapes_CreateBox(extends));
        }

        /// <summary>
        /// Create a sphere shape and retreive its handle
        /// </summary>
        /// <param name="radius">Radius of the sphere</param>
        /// <returns>Shape handle</returns>
        public static Shape Sphere(float radius)
        {
            return new Shape(Shapes_CreateSphere(radius));
        }

        /// <summary>
        /// Create a capsule shape and retreive its handle
        /// </summary>
        /// <param name="halfHeight">Half height of the cylinder part of the capsule</param>
        /// <param name="radius">Radius of the capsule</param>
        /// <returns>Shape handle</returns>
        public static Shape Capsule(float halfHeight, float radius)
        {
            return new Shape(Shapes_CreateCapsule(halfHeight, radius));
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Shapes_CreateBox(Vector3 extends);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Shapes_CreateSphere(float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Shapes_CreateCapsule(float halfHeight, float radius);

        #endregion
    }
}
