using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public enum ActivationType
    {
        /* Activate the body */
        Activate,
		/* Leave activation state as is */
		DontActivate
    };

    public enum BodyType
	{
		Static,
		Kinematic,
		Dynamic
	};

    public struct Ray
	{
		public Ray(Vector3 origin, Vector3 direction)
		{
			Origin = origin;
			Direction = direction;
		}

		public Vector3 Origin;
		public Vector3 Direction;
	}

    public struct RayCastHit
    {
        public float Distance;
        public UInt32 BodyID;
        public UInt32 SubShapeID;
    };

    public struct RayCastResult
    {
        public List<RayCastHit> Hits;
	};

    public static class Physics
    {
		public static bool CastRay(Ray ray, out RayCastResult result, float maxDistance = float.MaxValue, bool debugDraw = false)
		{
			result = new RayCastResult();
			RayCastHit[] hits = Physics_CastRay(ray.Origin, ray.Direction, maxDistance, debugDraw);
			if(hits == null || hits.Length == 0 ) return false;
			result.Hits = new List<RayCastHit>();
			for (int i = 0; i < hits.Length; i++)
			{
				result.Hits.Add(hits[i]);
			}
			return true;
        }

		public static Vector3 Gravity
		{
			get => Physics_GetGravity();
			set => Physics_SetGravity(value);
        }

		/* Ray Casting */
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RayCastHit[] Physics_CastRay(Vector3 origin, Vector3 direction, float maxDistance, bool debugDraw);

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
