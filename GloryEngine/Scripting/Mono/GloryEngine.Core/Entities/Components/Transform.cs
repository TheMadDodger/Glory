using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a Transform component
    /// </summary>
    public class Transform : NativeComponent
    {
        #region Props

        /// <summary>
        /// The position of the Entity in parent space
        /// </summary>
        public Vector3 LocalPosition
        {
            get => Transform_GetLocalPosition(Object.Scene.ID, Object.ID, _objectID);
            set => Transform_SetLocalPosition(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The Quaternion rotation of the Entity in parent space
        /// </summary>
        public Quaternion LocalRotation
        {
            get => Transform_GetLocalRotation(Object.Scene.ID, Object.ID, _objectID);
            set => Transform_SetLocalRotation(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The Euler rotation of the Entity in parent space
        /// </summary>
        public Vector3 LocalRotationEuler
        {
            get => Transform_GetLocalRotationEuler(Object.Scene.ID, Object.ID, _objectID);
            set => Transform_SetLocalRotationEuler(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The scale of the Entity in parent space
        /// </summary>
        public Vector3 LocalScale
        {
            get => Transform_GetLocalScale(Object.Scene.ID, Object.ID, _objectID);
            set => Transform_SetLocalScale(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The direction the z axis of the Entity is facing
        /// </summary>
        public Vector3 Forward
        {
            get => Transform_GetForward(Object.Scene.ID, Object.ID, _objectID);
            set => Transform_SetForward(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The direction the x axis of the Entity is facing
        /// </summary>
        public Vector3 Right => Transform_GetRight(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// The direction the y axis of the Entity is facing
        /// </summary>
        public Vector3 Up => Transform_GetUp(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// Transform matrix of the Entity in world space
        /// </summary>
        public Mat4 World => Transform_GetWorld(Object.Scene.ID, Object.ID, _objectID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Quaternion Transform_GetLocalRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Quaternion position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalRotationEuler(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalRotationEuler(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetForward(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetForward(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 forward);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetRight(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetUp(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Mat4 Transform_GetWorld(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        #endregion
    }
}
