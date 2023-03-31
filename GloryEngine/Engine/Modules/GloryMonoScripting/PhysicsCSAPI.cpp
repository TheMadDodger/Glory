#include "PhysicsCSAPI.h"
#include "MathCSAPI.h"
#include "GloryMonoScipting.h"

#include <PhysicsModule.h>

#define PHYSICS Game::GetGame().GetEngine()->GetPhysicsModule()

namespace Glory
{
#pragma region States

	void Physics_ActivateBody(uint32_t bodyID)
	{
		PHYSICS->ActivateBody(bodyID);
	}

	void Physics_DeactivateBody(uint32_t bodyID)
	{
		PHYSICS->DeactivateBody(bodyID);
	}

	bool Physics_IsBodyActive(uint32_t bodyID)
	{
		return PHYSICS->IsBodyActive(bodyID);
	}

	bool Physics_IsValidBody(uint32_t bodyID)
	{
		return PHYSICS->IsValidBody(bodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void Physics_SetBodyPosition(uint32_t bodyID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		PHYSICS->SetBodyPosition(bodyID, ToGLMVec3(*position), activationType);
	}

	void Physics_SetBodyRotation(uint32_t bodyID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		PHYSICS->SetBodyRotation(bodyID, ToGLMQuat(*rotation), activationType);
	}

	void Physics_SetBodyScale(uint32_t bodyID, const Vec3Wrapper* scale, const ActivationType activationType)
	{
		PHYSICS->SetBodyScale(bodyID, ToGLMVec3(*scale), activationType);
	}

	Vec3Wrapper Physics_GetBodyPosition(uint32_t bodyID)
	{
		return PHYSICS->GetBodyPosition(bodyID);
	}

	Vec3Wrapper Physics_GetBodyCenterOfMassPosition(uint32_t bodyID)
	{
		return PHYSICS->GetBodyCenterOfMassPosition(bodyID);
	}

	QuatWrapper Physics_GetBodyRotation(uint32_t bodyID)
	{
		return PHYSICS->GetBodyRotation(bodyID);
	}
	
#pragma endregion

#pragma region Velocities

	void Physics_MoveBodyKinematic(uint32_t bodyID, const Vec3Wrapper* targetPosition, const QuatWrapper* targetRotation, float deltaTime)
	{
		PHYSICS->MoveBodyKinematic(bodyID, ToGLMVec3(*targetPosition), ToGLMQuat(*targetRotation), deltaTime);
	}

	void Physics_SetBodyLinearAndAngularVelocity(uint32_t bodyID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PHYSICS->SetBodyLinearAndAngularVelocity(bodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void Physics_GetBodyLinearAndAngularVelocity(uint32_t bodyID, Vec3Wrapper* linearVelocity, Vec3Wrapper* angularVelocity)
	{
		glm::vec3 lv, av;
		PHYSICS->GetBodyLinearAndAngularVelocity(bodyID, lv, av);
		*linearVelocity = lv;
		*angularVelocity = av;
	}

	void Physics_SetBodyLinearVelocity(uint32_t bodyID, const Vec3Wrapper* linearVelocity)
	{
		PHYSICS->SetBodyLinearVelocity(bodyID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper Physics_GetBodyLinearVelocity(uint32_t bodyID)
	{
		return PHYSICS->GetBodyLinearVelocity(bodyID);
	}

	void Physics_AddBodyLinearVelocity(uint32_t bodyID, const Vec3Wrapper* linearVelocity)
	{
		PHYSICS->AddBodyLinearVelocity(bodyID, ToGLMVec3(*linearVelocity));
	}

	void Physics_AddBodyLinearAndAngularVelocity(uint32_t bodyID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PHYSICS->AddBodyLinearAndAngularVelocity(bodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void Physics_SetBodyAngularVelocity(uint32_t bodyID, const Vec3Wrapper* angularVelocity)
	{
		PHYSICS->SetBodyAngularVelocity(bodyID, ToGLMVec3(*angularVelocity));
	}

	Vec3Wrapper Physics_GetBodyAngularVelocity(uint32_t bodyID)
	{
		return PHYSICS->GetBodyAngularVelocity(bodyID);
	}

	Vec3Wrapper Physics_GetBodyPointVelocity(uint32_t bodyID, const Vec3Wrapper* point)
	{
		return PHYSICS->GetBodyPointVelocity(bodyID, ToGLMVec3(*point));
	}

	void Physics_SetBodyPositionRotationAndVelocity(uint32_t bodyID, const Vec3Wrapper* position, const QuatWrapper* rotation, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PHYSICS->SetBodyPositionRotationAndVelocity(bodyID, ToGLMVec3(*position), ToGLMQuat(*rotation), ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

#pragma endregion

#pragma region Forces

	void Physics_AddBodyForce(uint32_t bodyID, const Vec3Wrapper* force)
	{
		PHYSICS->AddBodyForce(bodyID, ToGLMVec3(*force));
	}

	void Physics_AddBodyForce_Point(uint32_t bodyID, const Vec3Wrapper* force, const Vec3Wrapper* point)
	{
		PHYSICS->AddBodyForce(bodyID, ToGLMVec3(*force), ToGLMVec3(*point));
	}

	void Physics_AddBodyTorque(uint32_t bodyID, const Vec3Wrapper* torque)
	{
		PHYSICS->AddBodyTorque(bodyID, ToGLMVec3(*torque));
	}

	void Physics_AddBodyForceAndTorque(uint32_t bodyID, const Vec3Wrapper* force, const Vec3Wrapper* torque)
	{
		PHYSICS->AddBodyForceAndTorque(bodyID, ToGLMVec3(*force), ToGLMVec3(*torque));
	}

#pragma endregion

#pragma region Impulses

	void Physics_AddBodyImpulse(uint32_t bodyID, const Vec3Wrapper* impulse)
	{
		PHYSICS->AddBodyImpulse(bodyID, ToGLMVec3(*impulse));
	}

	void Physics_AddBodyImpulse_Point(uint32_t bodyID, const Vec3Wrapper* impulse, const Vec3Wrapper* point)
	{
		PHYSICS->AddBodyForce(bodyID, ToGLMVec3(*impulse), ToGLMVec3(*point));
	}

	void Physics_AddBodyAngularImpulse(uint32_t bodyID, const Vec3Wrapper* angularImpulse)
	{
		PHYSICS->AddBodyAngularImpulse(bodyID, ToGLMVec3(*angularImpulse));
	}

#pragma endregion

#pragma region Bindings

	void PhysicsCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Body status */
		BIND("GloryEngine.Physics::Physics_ActivateBody", Physics_ActivateBody);
		BIND("GloryEngine.Physics::Physics_DeactivateBody", Physics_DeactivateBody);
		BIND("GloryEngine.Physics::Physics_IsBodyActive", Physics_IsBodyActive);
		BIND("GloryEngine.Physics::Physics_IsValidBody", Physics_IsValidBody);

		/* Position and rotation */
		BIND("GloryEngine.Physics::Physics_SetBodyPosition", Physics_SetBodyPosition);
		BIND("GloryEngine.Physics::Physics_SetBodyRotation", Physics_SetBodyRotation);
		BIND("GloryEngine.Physics::Physics_SetBodyScale", Physics_SetBodyScale);
		BIND("GloryEngine.Physics::Physics_GetBodyPosition", Physics_GetBodyPosition);
		BIND("GloryEngine.Physics::Physics_GetBodyCenterOfMassPosition", Physics_GetBodyCenterOfMassPosition);
		BIND("GloryEngine.Physics::Physics_GetBodyRotation", Physics_GetBodyRotation);

		/* Velocities */
		BIND("GloryEngine.Physics::Physics_MoveBodyKinematic", Physics_MoveBodyKinematic);
		BIND("GloryEngine.Physics::Physics_SetBodyLinearAndAngularVelocity", Physics_SetBodyLinearAndAngularVelocity);
		BIND("GloryEngine.Physics::Physics_GetBodyLinearAndAngularVelocity", Physics_GetBodyLinearAndAngularVelocity);
		BIND("GloryEngine.Physics::Physics_SetBodyLinearVelocity", Physics_SetBodyLinearVelocity);
		BIND("GloryEngine.Physics::Physics_GetBodyLinearVelocity", Physics_GetBodyLinearVelocity);
		BIND("GloryEngine.Physics::Physics_AddBodyLinearVelocity", Physics_AddBodyLinearVelocity);
		BIND("GloryEngine.Physics::Physics_AddBodyLinearAndAngularVelocity", Physics_AddBodyLinearAndAngularVelocity);
		BIND("GloryEngine.Physics::Physics_SetBodyAngularVelocity", Physics_SetBodyAngularVelocity);
		BIND("GloryEngine.Physics::Physics_GetBodyAngularVelocity", Physics_GetBodyAngularVelocity);
		BIND("GloryEngine.Physics::Physics_GetBodyPointVelocity", Physics_GetBodyPointVelocity);
		BIND("GloryEngine.Physics::Physics_SetBodyPositionRotationAndVelocity", Physics_SetBodyPositionRotationAndVelocity);

		/* Velocities */
		BIND("GloryEngine.Physics::Physics_AddBodyForce", Physics_AddBodyForce);
		BIND("GloryEngine.Physics::Physics_AddBodyForce_Point", Physics_AddBodyForce_Point);
		BIND("GloryEngine.Physics::Physics_AddBodyTorque", Physics_AddBodyTorque);
		BIND("GloryEngine.Physics::Physics_AddBodyForceAndTorque", Physics_AddBodyForceAndTorque);

		/* Impulses */
		BIND("GloryEngine.Physics::Physics_AddBodyImpulse", Physics_AddBodyImpulse);
		BIND("GloryEngine.Physics::Physics_AddBodyImpulse_Point", Physics_AddBodyImpulse_Point);
		BIND("GloryEngine.Physics::Physics_AddBodyAngularImpulse", Physics_AddBodyAngularImpulse);
	}

#pragma endregion
}
