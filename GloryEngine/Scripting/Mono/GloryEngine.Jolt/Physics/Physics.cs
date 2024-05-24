using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Activation type
    /// </summary>
    public enum ActivationType
    {
        /// <summary>
        /// Activate the body
        /// </summary>
        Activate,
        /// <summary>
        /// Leave activation state as is
        /// </summary>
        DontActivate
    };

    /// <summary>
    /// Body type
    /// </summary>
    public enum BodyType
	{
		/// <summary>
		/// Static body
		/// </summary>
		Static,
		/// <summary>
		/// Kinematic body
		/// </summary>
		Kinematic,
		/// <summary>
		/// Dynamic body
		/// </summary>
		Dynamic
	};

	/// <summary>
	/// Ray
	/// </summary>
    public struct Ray
	{
		/// <summary>
		/// Construct a ray
		/// </summary>
		/// <param name="origin">Origin</param>
		/// <param name="direction">Direction</param>
		public Ray(Vector3 origin, Vector3 direction)
		{
			Origin = origin;
			Direction = direction;
		}

		/// <summary>
		/// Origin
		/// </summary>
		public Vector3 Origin;
		/// <summary>
		/// Direction
		/// </summary>
		public Vector3 Direction;
	}

	/// <summary>
	/// Raycast hit
	/// </summary>
    public struct RayCastHit
    {
		/// <summary>
		/// Distance from the origin of the ray
		/// </summary>
        public float Distance;
		/// <summary>
		/// ID of the physics body that was hit
		/// </summary>
        public UInt32 BodyID;
		/// <summary>
		/// ID of the subshape that was hit
		/// </summary>
        public UInt32 SubShapeID;
		/// <summary>
		/// The position on which the hit occured
		/// </summary>
		public Vector3 Pos;
    };

	/// <summary>
	/// Hit result for a raycast
	/// </summary>
    public struct RayCastResult
    {
		/// <summary>
		/// List containing all hits of a raycast
		/// </summary>
        public List<RayCastHit> Hits;
	};

	/// <summary>
	/// Physics API
	/// </summary>
    public static class Physics
    {
        /// <summary>
        /// Do a raycast check.
        /// 
        /// Whenever there are any hits, result will always have a list with a size of maxHits.
		/// Use the returned integer value to determine the actual hit count.
        /// </summary>
        /// <param name="ray">Origin and direction ray</param>
        /// <param name="result">The out result of the raycast</param>
        /// <param name="maxDistance">Maximum distance an object can be considered for the raycast</param>
		/// <param name="layerMask">Mask with layers to filter the raycast with</param>
        /// <param name="debugDraw">Wether to render debug information on the raycast</param>
		/// <param name="ignoreBodies">Array of physics bodies by ID to ignore in the raycast</param>
        /// <param name="maxHits">The maximum number of hits the raycast may produce</param>
        /// <returns>The number of hit colliders during the raycast</returns>
        public static int CastRay(Ray ray, out RayCastResult result, float maxDistance = 9999.9f, LayerMask layerMask = new LayerMask(), UInt32[] ignoreBodies = null, bool debugDraw = false, int maxHits = 20)
		{
			result = new RayCastResult();
			RayCastHit[] hits = new RayCastHit[maxHits];
			int numHits = Physics_CastRayNoAlloc(ray.Origin, ray.Direction, maxDistance, layerMask, ignoreBodies, debugDraw, hits);
			if(numHits == 0) return 0;
			result.Hits = new List<RayCastHit>(hits);
			return numHits;
        }

		/// <summary>
		/// Physics gravity
		/// </summary>
		public static Vector3 Gravity
		{
			get => Physics_GetGravity();
			set => Physics_SetGravity(value);
        }

		/* Ray Casting */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RayCastHit[] Physics_CastRay(Vector3 origin, Vector3 direction, float maxDistance, LayerMask layerMask, UInt32[] ignoreBodies, bool debugDraw);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Physics_CastRayNoAlloc(Vector3 origin, Vector3 direction, float maxDistance, LayerMask layerMask, UInt32[] ignoreBodies, bool debugDraw, RayCastHit[] hits);

        /* Gravity */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Physics_SetGravity(Vector3 gravity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 Physics_GetGravity();

        /* States */
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_ActivateBody(UInt32 bodyID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_DeactivateBody(UInt32 bodyID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Physics_IsBodyActive(UInt32 bodyID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Physics_IsValidBody(UInt32 bodyID);

		/* Position and rotation */
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyPosition(UInt32 bodyID, ref Vector3 position, ActivationType activationType = ActivationType.Activate);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyRotation(UInt32 bodyID, ref Quaternion rotation, ActivationType activationType = ActivationType.Activate);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyScale(UInt32 bodyID, ref Vector3 scale, ActivationType activationType = ActivationType.Activate);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Vector3 Physics_GetBodyPosition(UInt32 bodyID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Vector3 Physics_GetBodyCenterOfMassPosition(UInt32 bodyID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Quaternion Physics_GetBodyRotation(UInt32 bodyID);

		/* Velocities */
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_MoveBodyKinematic(UInt32 bodyID, ref Vector3 targetPosition, ref Quaternion targetRotation, float deltaTime);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyLinearAndAngularVelocity(UInt32 bodyID, ref Vector3 linearVelocity, ref Vector3 angularVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_GetBodyLinearAndAngularVelocity(UInt32 bodyID, ref Vector3 linearVelocity, ref Vector3 angularVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyLinearVelocity(UInt32 bodyID, ref Vector3 linearVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Vector3 Physics_GetBodyLinearVelocity(UInt32 bodyID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyLinearVelocity(UInt32 bodyID, ref Vector3 linearVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyLinearAndAngularVelocity(UInt32 bodyID, ref Vector3 linearVelocity, ref Vector3 angularVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyAngularVelocity(UInt32 bodyID, ref Vector3 angularVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Vector3 Physics_GetBodyAngularVelocity(UInt32 bodyID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Vector3 Physics_GetBodyPointVelocity(UInt32 bodyID, ref Vector3 point);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_SetBodyPositionRotationAndVelocity(UInt32 bodyID, ref Vector3 position, ref Quaternion rotation, ref Vector3 linearVelocity, ref Vector3 angularVelocity);

		/* Forces */
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyForce(UInt32 bodyID, ref Vector3 force);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyForce_Point(UInt32 bodyID, ref Vector3 force, ref Vector3 point);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyTorque(UInt32 bodyID, ref Vector3 torque);
        [MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyForceAndTorque(UInt32 bodyID, ref Vector3 force, ref Vector3 torque);

		/* Impulses */
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyImpulse(UInt32 bodyID, ref Vector3 impulse);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyImpulse_Point(UInt32 bodyID, ref Vector3 impulse, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Physics_AddBodyAngularImpulse(UInt32 bodyID, ref Vector3 angularImpulse);
    }
}
