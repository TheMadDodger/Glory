using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a CharacterController component
    /// </summary>
    public class CharacterController : EntityComponent
    {
        #region Props

        /// <summary>
        /// Internal ID of the character
        /// </summary>
        public UInt32 CharacterID => CharacterController_GetCharacterID(ref _entity, _objectID);

        /// <summary>
        /// Internal ID of the Physics body of the character managed by the PhysicsModule
        /// </summary>
        public UInt32 BodyID => CharacterController_GetBodyID(ref _entity, _objectID);

        /// <summary>
        /// Whether the character is active and awake
        /// </summary>
        public bool BodyActive
        {
            set
            {
                if (value)
                {
                    CharacterController_Activate(ref _entity, _objectID);
                    return;
                }
                CharacterController_Deactivate(ref _entity, _objectID);
            }
            get => CharacterController_IsActive(ref _entity, _objectID);
        }

        /// <summary>
        /// Default activation type to pass to function calls
        /// </summary>
        public ActivationType DefaultActivationType = ActivationType.Activate;

        /// <summary>
        /// Position of the character
        /// Use this instead of Transform.Position
        /// </summary>
        public Vector3 Position
        {
            get => CharacterController_GetPosition(ref _entity, _objectID);
            set => CharacterController_SetPosition(ref _entity, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Rotation of the character
        /// Use this instead of Transform.Rotation
        /// </summary>
        public Quaternion Rotation
        {
            get => CharacterController_GetRotation(ref _entity, _objectID);
            set => CharacterController_SetRotation(ref _entity, _objectID, ref value, DefaultActivationType);
        }

        /// <summary>
        /// Position of the center of mass of the character
        /// </summary>
        public Vector3 CenterOfMassPosition => CharacterController_GetCenterOfMassPosition(ref _entity, _objectID);

        /// <summary>
        /// The current linear velocity of the character
        /// </summary>
        public Vector3 LinearVelocity
        {
            get => CharacterController_GetLinearVelocity(ref _entity, _objectID);
            set => CharacterController_SetLinearVelocity(ref _entity, _objectID, ref value);
        }

        /// <summary>
        /// The current angular velocity of the character
        /// </summary>
        public Vector3 AngularVelocity
        {
            get => CharacterController_GetAngularVelocity(ref _entity, _objectID);
            set => CharacterController_SetAngularVelocity(ref _entity, _objectID, ref value);
        }

        public Shape Shape
        {
            get => new Shape(CharacterController_GetShapeID(ref _entity, _objectID));
            set => CharacterController_SetShape(ref _entity, _objectID, value.ShapeID);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Add an impulse to the character
        /// </summary>
        /// <param name="impulse">Direction and amplitude of the impulse</param>
        public void AddImpulse(ref Vector3 impulse) => CharacterController_AddImpulse(ref _entity, _objectID, ref impulse);
        /// <summary>
        /// Add a vector to the linear velocity of the character
        /// </summary>
        /// <param name="linearVelocity">Direction and amplitude of the velocity to add</param>
        public void AddLinearVelocity(ref Vector3 linearVelocity) => CharacterController_AddLinearVelocity(ref _entity, _objectID, ref linearVelocity);

        /// <summary>
        /// Set the shape of the character controller
        /// </summary>
        /// <param name="shape">Shape to set</param>
        /// <param name="maxPenetrationDepth">Checks if the new shapes collides with a given distance, set to float.MaxValue to ignore</param>
        /// <param name="lockBodies">Whether to lock the body while setting the shape</param>
        /// <returns>True if the switch succeeded</returns>
        public bool SetShape(Shape shape, float maxPenetrationDepth = float.MaxValue, bool lockBodies = true)
        {
            return CharacterController_SetShape(ref _entity, _objectID, shape.ShapeID, maxPenetrationDepth, lockBodies);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 CharacterController_GetCharacterID(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 CharacterController_GetBodyID(ref Entity entity, UInt64 componentID);

        /* States */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_Activate(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_Deactivate(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool CharacterController_IsActive(ref Entity entity, UInt64 componentID);

        /* Position and rotation */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 CharacterController_GetPosition(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_SetPosition(ref Entity entity, UInt64 componentID, ref Vector3 position, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Quaternion CharacterController_GetRotation(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_SetRotation(ref Entity entity, UInt64 componentID, ref Quaternion rotation, ActivationType activationType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 CharacterController_GetCenterOfMassPosition(ref Entity entity, UInt64 componentID);

        /* Velocities */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 CharacterController_GetLinearVelocity(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_SetLinearVelocity(ref Entity entity, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_AddLinearVelocity(ref Entity entity, UInt64 componentID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 CharacterController_GetAngularVelocity(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_SetAngularVelocity(ref Entity entity, UInt64 componentID, ref Vector3 angularVelocity);

        /* Impulses */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CharacterController_AddImpulse(ref Entity entity, UInt64 componentID, ref Vector3 impulse);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 CharacterController_GetShapeID(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool CharacterController_SetShape(ref Entity entity, UInt64 componentID, UInt64 shapeID, float maxPenetrationDepth = float.MaxValue, bool lockBodies = true);

        #endregion
    }
}
