using GloryEngine.SceneManagement;
using System;

namespace GloryEngine
{
    /// <summary>
    /// Internal data structure for picking results
    /// </summary>
    internal struct PickResultInternal
    {
        /// <summary>
        /// Camera that did the picking
        /// </summary>
        public UInt64 CameraID;
        /// <summary>
        /// Object ID that was picked, or null of no object was picked
        /// </summary>
        public UInt64 ObjectID;
        /// <summary>
        /// World position of the pick
        /// </summary>
        public Vector3 Position;
        /// <summary>
        /// Normal at the pick position
        /// </summary>
        public Vector3 Normal;
    }

    /// <summary>
    /// Data structure for picking results
    /// </summary>
    public struct PickResult
    {
        /// <summary>
        /// Camera that did the picking
        /// </summary>
        public UInt64 CameraID;
        /// <summary>
        /// Object that was picked, or null of no object was picked
        /// </summary>
        public SceneObject Object;
        /// <summary>
        /// World position of the pick
        /// </summary>
        public Vector3 Position;
        /// <summary>
        /// Normal at the pick position
        /// </summary>
        public Vector3 Normal;
    }
}
