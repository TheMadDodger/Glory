using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a Transform component
    /// </summary>
    public class Transform : EntityComponent
    {
        #region Props

        /// <summary>
        /// The position of the Entity in parent space
        /// </summary>
        public Vector3 LocalPosition
        {
            get => Transform_GetLocalPosition(ref _entity, _objectID);
            set => Transform_SetLocalPosition(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The Quaternion rotation of the Entity in parent space
        /// </summary>
        public Quaternion LocalRotation
        {
            get => Transform_GetLocalRotation(ref _entity, _objectID);
            set => Transform_SetLocalRotation(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The Euler rotation of the Entity in parent space
        /// </summary>
        public Vector3 LocalRotationEuler
        {
            get => Transform_GetLocalRotationEuler(ref _entity, _objectID);
            set => Transform_SetLocalRotationEuler(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The scale of the Entity in parent space
        /// </summary>
        public Vector3 LocalScale
        {
            get => Transform_GetLocalScale(ref _entity, _objectID);
            set => Transform_SetLocalScale(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The direction the z axis of the Entity is facing
        /// </summary>
        public Vector3 Forward
        {
            get => Transform_GetForward(ref _entity, _objectID);
            set => Transform_SetForward(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The direction the x axis of the Entity is facing
        /// </summary>
        public Vector3 Right => Transform_GetRight(ref _entity, _objectID);

        /// <summary>
        /// The direction the y axis of the Entity is facing
        /// </summary>
        public Vector3 Up => Transform_GetUp(ref _entity, _objectID);

        /// <summary>
        /// Transform matrix of the Entity in world space
        /// </summary>
        public Mat4 World => Transform_GetWorld(ref _entity, _objectID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalPosition(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalPosition(ref Entity entity, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Quaternion Transform_GetLocalRotation(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalRotation(ref Entity entity, UInt64 componentID, ref Quaternion position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalRotationEuler(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalRotationEuler(ref Entity entity, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetLocalScale(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalScale(ref Entity entity, UInt64 componentID, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetForward(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetForward(ref Entity entity, UInt64 componentID, ref Vector3 forward);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetRight(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector3 Transform_GetUp(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Mat4 Transform_GetWorld(ref Entity entity, UInt64 componentID);

        #endregion
    }
}
