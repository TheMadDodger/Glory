using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a PhysicsBody component
    /// </summary>
    public class PhysicsBody : NativeComponent
    {
        #region Props

        /// <summary>
        /// Internal ID of the Physics managed by the PhysicsModule
        /// </summary>
        public UInt32 BodyID => PhysicsBody_GetID(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// Whether the Physics is active and awake
        /// </summary>
        public bool BodyActive
        {
            set
            {
                if(value)
                {
                    PhysicsBody_Activate(Object.Scene.ID, Object.ID, _objectID);
                    return;
                }
                PhysicsBody_Deactivate(Object.Scene.ID, Object.ID, _objectID);
            }
            get => PhysicsBody_IsActive(Object.Scene.ID, Object.ID, _objectID);
        }

        /// <summary>
        /// Whether this component has a valid Physics
        /// </summary>
        public bool Valid => PhysicsBody_IsValid(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// Body type
        /// </summary>
        public BodyType BodyType
        {
            get => PhysicsBody_GetBodyType(Object.Scene.ID, Object.ID, _objectID);
            set => PhysicsBody_SetBodyType(Object.Scene.ID, Object.ID, _objectID, value, DefaultActivationType);
        }

        /// <summary>
        /// Default activation type to pass to function calls
        /// </summary>
        public ActivationType DefaultActivationType = ActivationType.Activate;

        /// <summary>
        /// Position of the Physics
        /// Use this instead of Transform.Position
        /// </summary>
        public Vector3 Position
        {
            get => PhysicsBody_GetPosition(Object.Scene.ID, Object.ID, _objectID);
            set => PhysicsBody_SetPosition(Object.Scene.ID, Object.ID, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Rotation of the Physics
        /// Use this instead of Transform.Rotation
        /// </summary>
        public Quaternion Rotation
        {
            get => PhysicsBody_GetRotation(Object.Scene.ID, Object.ID, _objectID);
            set => PhysicsBody_SetRotation(Object.Scene.ID, Object.ID, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Scale of the Physics
        /// Use this together with Transform.Scale
        /// </summary>
        public Vector3 Scale
        {
            set => PhysicsBody_SetScale(Object.Scene.ID, Object.ID, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Position of the center of mass of the Physics
        /// </summary>
        public Vector3 CenterOfMassPosition => PhysicsBody_GetCenterOfMassPosition(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// The current linear velocity of the Physics
        /// </summary>
        public Vector3 LinearVelocity
        {
            get => PhysicsBody_GetLinearVelocity(Object.Scene.ID, Object.ID, _objectID);
            set => PhysicsBody_SetLinearVelocity(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        /// <summary>
        /// The current angular velocity of the Physics
        /// </summary>
        public Vector3 AngularVelocity
        {
            get => PhysicsBody_GetAngularVelocity(Object.Scene.ID, Object.ID, _objectID);
            set => PhysicsBody_SetAngularVelocity(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Set velocity of  such that it will be positioned at inTargetPosition/Rotation in inDeltaTime seconds (will activate  if needed)
        /// </summary>
        /// <param name="targetPosition">Position to reach</param>
        /// <param name="targetRotation">Desired rotation</param>
        /// <param name="deltaTime">Time to get to the desired state</param>
        public void MoveKinematic(ref Vector3 targetPosition, ref Quaternion targetRotation, float deltaTime) => PhysicsBody_MoveKinematic(Object.Scene.ID, Object.ID, _objectID, ref targetPosition, ref targetRotation, deltaTime);

        /// <summary>
        /// Add a force to the Physics
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        public void AddForce(ref Vector3 force) => PhysicsBody_AddForce(Object.Scene.ID, Object.ID, _objectID, ref force);
        /// <summary>
        /// Add a force to the Physics from a point
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        /// <param name="point">Origin of the applied force</param>
        public void AddForce(ref Vector3 force, ref Vector3 point) => PhysicsBody_AddForce_Point(Object.Scene.ID, Object.ID, _objectID, ref force, ref point);
        /// <summary>
        /// Add a torque to the Physics
        /// </summary>
        /// <param name="torque">Direction and amplitude of the torque to add</param>
        public void AddTorque(ref Vector3 torque) => PhysicsBody_AddTorque(Object.Scene.ID, Object.ID, _objectID, ref torque);
        /// <summary>
        /// Add both force and torque to the Physics
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        /// <param name="torque">Direction and amplitude of the torque</param>
        public void AddForceAndTorque(ref Vector3 force, ref Vector3 torque) => PhysicsBody_AddForceAndTorque(Object.Scene.ID, Object.ID, _objectID, ref force, ref torque);

        /// <summary>
        /// Add an impulse to the Physics
        /// </summary>
        /// <param name="impulse">Direction and amplitude of the impulse</param>
        public void AddImpulse(ref Vector3 impulse) => PhysicsBody_AddImpulse(Object.Scene.ID, Object.ID, _objectID, ref impulse);
        /// <summary>
        /// Add a force to the Physics from a point
        /// </summary>
        /// <param name="impulse">Direction and amplitude of the impulse</param>
        /// <param name="point">Origin of the applied impulse</param>
        public void AddImpulse(ref Vector3 impulse, ref Vector3 point) => PhysicsBody_AddImpulse_Point(Object.Scene.ID, Object.ID, _objectID, ref impulse, ref point);
        /// <summary>
        /// Add an angular impulse to the Physics
        /// </summary>
        /// <param name="angularImpulse">Direction and amplitude of the impulse</param>
        public void AddAngularImpulse(ref Vector3 angularImpulse) => PhysicsBody_AddAngularImpulse(Object.Scene.ID, Object.ID, _objectID, ref angularImpulse);
        /// <summary>
        /// Add a vector to the linear velocity of the Physics
        /// </summary>
        /// <param name="linearVelocity">Direction and amplitude of the velocity to add</param>
        public void AddLinearVelocity(ref Vector3 linearVelocity) => PhysicsBody_AddLinearVelocity(Object.Scene.ID, Object.ID, _objectID, ref linearVelocity);
        /// <summary>
        /// Get the velocity of the Physics from a point
        /// </summary>
        /// <param name="point">Origin of the velocity</param>
        public Vector3 GetPointVelocity(ref Vector3 point) => PhysicsBody_GetPointVelocity(Object.Scene.ID, Object.ID, _objectID, ref point);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 PhysicsBody_GetID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        /* States */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_Activate(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_Deactivate(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool PhysicsBody_IsActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool PhysicsBody_IsValid(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern BodyType PhysicsBody_GetBodyType(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetBodyType(UInt64 sceneID, UInt64 objectID, UInt64 componentID, BodyType bodyType, ActivationType activationType);

        /* Position and rotation */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 position, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Quaternion PhysicsBody_GetRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Quaternion rotation, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 scale, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetCenterOfMassPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        /* Velocities */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_MoveKinematic(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 targetPosition, ref Quaternion targetRotation, float deltaTime);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetLinearVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetLinearVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddLinearVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetAngularVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetAngularVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 angularVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetPointVelocity(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 point);

        /* Forces */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForce(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 force);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForce_Point(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 force, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddTorque(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 torque);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForceAndTorque(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 force, ref Vector3 torque);

        /* Impulses */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddImpulse(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 impulse);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddImpulse_Point(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 impulse, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddAngularImpulse(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector3 angularImpulse);

        #endregion
    }
}
