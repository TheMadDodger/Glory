using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a PhysicsBody component
    /// </summary>
    public class PhysicsBody : EntityComponent
    {
        #region Props

        /// <summary>
        /// Internal ID of the Physics managed by the PhysicsModule
        /// </summary>
        public UInt32 BodyID => PhysicsBody_GetID(ref _entity, _objectID);

        /// <summary>
        /// Whether the Physics is active and awake
        /// </summary>
        public bool Active
        {
            set
            {
                if(value)
                {
                    PhysicsBody_Activate(ref _entity, _objectID);
                    return;
                }
                PhysicsBody_Deactivate(ref _entity, _objectID);
            }
            get => PhysicsBody_IsActive(ref _entity, _objectID);
        }

        /// <summary>
        /// Whether this component has a valid Physics
        /// </summary>
        public bool Valid => PhysicsBody_IsValid(ref _entity, _objectID);

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
            get => PhysicsBody_GetPosition(ref _entity, _objectID);
            set => PhysicsBody_SetPosition(ref _entity, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Rotation of the Physics
        /// Use this instead of Transform.Rotation
        /// </summary>
        public Quaternion Rotation
        {
            get => PhysicsBody_GetRotation(ref _entity, _objectID);
            set => PhysicsBody_SetRotation(ref _entity, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Scale of the Physics
        /// Use this together with Transform.Scale
        /// </summary>
        public Vector3 Scale
        {
            set => PhysicsBody_SetScale(ref _entity, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Position of the center of mass of the Physics
        /// </summary>
        public Vector3 CenterOfMassPosition => PhysicsBody_GetCenterOfMassPosition(ref _entity, _objectID);

        /// <summary>
        /// The current linear velocity of the Physics
        /// </summary>
        public Vector3 LinearVelocity
        {
            get => PhysicsBody_GetLinearVelocity(ref _entity, _objectID);
            set => PhysicsBody_SetLinearVelocity(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The current angular velocity of the Physics
        /// </summary>
        public Vector3 AngularVelocity
        {
            get => PhysicsBody_GetAngularVelocity(ref _entity, _objectID);
            set => PhysicsBody_SetAngularVelocity(ref _entity, _objectID, ref value);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Set velocity of  such that it will be positioned at inTargetPosition/Rotation in inDeltaTime seconds (will activate  if needed)
        /// </summary>
        /// <param name="targetPosition">Position to reach</param>
        /// <param name="targetRotation">Desired rotation</param>
        /// <param name="deltaTime">Time to get to the desired state</param>
        public void MoveKinematic(ref Vector3 targetPosition, ref Quaternion targetRotation, float deltaTime) => PhysicsBody_MoveKinematic(ref _entity, _objectID, ref targetPosition, ref targetRotation, deltaTime);

        /// <summary>
        /// Add a force to the Physics
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        public void AddForce(ref Vector3 force) => PhysicsBody_AddForce(ref _entity, _objectID, ref force);
        /// <summary>
        /// Add a force to the Physics from a point
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        /// <param name="point">Origin of the applied force</param>
        public void AddForce(ref Vector3 force, ref Vector3 point) => PhysicsBody_AddForce_Point(ref _entity, _objectID, ref force, ref point);
        /// <summary>
        /// Add a torque to the Physics
        /// </summary>
        /// <param name="torque">Direction and amplitude of the torque to add</param>
        public void AddTorque(ref Vector3 torque) => PhysicsBody_AddTorque(ref _entity, _objectID, ref torque);
        /// <summary>
        /// Add both force and torque to the Physics
        /// </summary>
        /// <param name="force">Direction and amplitude of the force</param>
        /// <param name="torque">Direction and amplitude of the torque</param>
        public void AddForceAndTorque(ref Vector3 force, ref Vector3 torque) => PhysicsBody_AddForceAndTorque(ref _entity, _objectID, ref force, ref torque);

        /// <summary>
        /// Add an impulse to the Physics
        /// </summary>
        /// <param name="impulse">Direction and amplitude of the impulse</param>
        public void AddImpulse(ref Vector3 impulse) => PhysicsBody_AddImpulse(ref _entity, _objectID, ref impulse);
        /// <summary>
        /// Add a force to the Physics from a point
        /// </summary>
        /// <param name="impulse">Direction and amplitude of the impulse</param>
        /// <param name="point">Origin of the applied impulse</param>
        public void AddImpulse(ref Vector3 impulse, ref Vector3 point) => PhysicsBody_AddImpulse_Point(ref _entity, _objectID, ref impulse, ref point);
        /// <summary>
        /// Add an angular impulse to the Physics
        /// </summary>
        /// <param name="angularImpulse">Direction and amplitude of the impulse</param>
        public void AddAngularImpulse(ref Vector3 angularImpulse) => PhysicsBody_AddAngularImpulse(ref _entity, _objectID, ref angularImpulse);
        /// <summary>
        /// Add a vector to the linear velocity of the Physics
        /// </summary>
        /// <param name="linearVelocity">Direction and amplitude of the velocity to add</param>
        public void AddLinearVelocity(ref Vector3 linearVelocity) => PhysicsBody_AddLinearVelocity(ref _entity, _objectID, ref linearVelocity);
        /// <summary>
        /// Get the velocity of the Physics from a point
        /// </summary>
        /// <param name="point">Origin of the velocity</param>
        public Vector3 GetPointVelocity(ref Vector3 point) => PhysicsBody_GetPointVelocity(ref _entity, _objectID, ref point);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 PhysicsBody_GetID(ref Entity entity, UInt64 componentID);

        /* States */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_Activate(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_Deactivate(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool PhysicsBody_IsActive(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool PhysicsBody_IsValid(ref Entity entity, UInt64 componentID);

        /* Position and rotation */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetPosition(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetPosition(ref Entity entity, UInt64 componentID, ref Vector3 position, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Quaternion PhysicsBody_GetRotation(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetRotation(ref Entity entity, UInt64 componentID, ref Quaternion rotation, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetScale(ref Entity entity, UInt64 componentID, ref Vector3 scale, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetCenterOfMassPosition(ref Entity entity, UInt64 componentID);

        /* Velocities */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_MoveKinematic(ref Entity entity, UInt64 componentID, ref Vector3 targetPosition, ref Quaternion targetRotation, float deltaTime);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetLinearVelocity(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetLinearVelocity(ref Entity entity, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddLinearVelocity(ref Entity entity, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetAngularVelocity(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_SetAngularVelocity(ref Entity entity, UInt64 componentID, ref Vector3 angularVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 PhysicsBody_GetPointVelocity(ref Entity entity, UInt64 componentID, ref Vector3 point);

        /* Forces */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForce(ref Entity entity, UInt64 componentID, ref Vector3 force);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForce_Point(ref Entity entity, UInt64 componentID, ref Vector3 force, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddTorque(ref Entity entity, UInt64 componentID, ref Vector3 torque);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddForceAndTorque(ref Entity entity, UInt64 componentID, ref Vector3 force, ref Vector3 torque);

        /* Impulses */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddImpulse(ref Entity entity, UInt64 componentID, ref Vector3 impulse);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddImpulse_Point(ref Entity entity, UInt64 componentID, ref Vector3 impulse, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PhysicsBody_AddAngularImpulse(ref Entity entity, UInt64 componentID, ref Vector3 angularImpulse);

        #endregion
    }
}
