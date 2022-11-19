using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class Transform : EntityComponent
    {
        #region Props

        public Vector3 LocalPosition
        {
            get => Transform_GetLocalPosition(ref _entity, _componentID);
            set => Transform_SetLocalPosition(ref _entity, _componentID, ref value);
        }

        public Quaternion LocalRotation
        {
            get => Transform_GetLocalRotation(ref _entity, _componentID);
            set => Transform_SetLocalRotation(ref _entity, _componentID, ref value);
        }

        public Vector3 LocalRotationEuler
        {
            get => Transform_GetLocalRotationEuler(ref _entity, _componentID);
            set => Transform_SetLocalRotationEuler(ref _entity, _componentID, ref value);
        }

        public Vector3 LocalScale
        {
            get => Transform_GetLocalScale(ref _entity, _componentID);
            set => Transform_SetLocalScale(ref _entity, _componentID, ref value);
        }

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

        #endregion
    }
}
