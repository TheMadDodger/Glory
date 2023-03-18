#pragma once
#include "Module.h"
#include "Physics.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Glory
{
    class PhysicsModule : public Module
    {
	public:
		PhysicsModule();
		virtual ~PhysicsModule();

		virtual const std::type_info& GetModuleType() override;

		virtual uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType) = 0;
		virtual void DestroyPhysicsBody(uint32_t& bodyID) = 0;
		virtual void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) = 0;
		virtual void SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual void ActivateBody(uint32_t bodyID) = 0;
		virtual void DeactivateBody(uint32_t bodyID) = 0;
		virtual bool IsBodyActive(uint32_t bodyID) const = 0;
		virtual bool IsValidBody(uint32_t bodyID) const = 0;

		/*
		RVec3 GetPosition(const BodyID& inBodyID) const;
		RVec3 GetCenterOfMassPosition(const BodyID& inBodyID) const;
		Quat GetRotation(const BodyID& inBodyID) const;
		RMat44 GetWorldTransform(const BodyID& inBodyID) const;
		RMat44 GetCenterOfMassTransform(const BodyID& inBodyID) const;

		/// Create a two body constraint
		TwoBodyConstraint* CreateConstraint(const TwoBodyConstraintSettings* inSettings, const BodyID& inBodyID1, const BodyID& inBodyID2);

		/// Activate non-static bodies attached to a constraint
		void ActivateConstraint(const TwoBodyConstraint* inConstraint);

		///@name Access to the shape of a body
		///@{

		///@}

		///@name Object layer of a body
		///@{
		void SetObjectLayer(const BodyID& inBodyID, ObjectLayer inLayer);
		ObjectLayer GetObjectLayer(const BodyID& inBodyID) const;
		///@}

		/// Set velocity of body such that it will be positioned at inTargetPosition/Rotation in inDeltaTime seconds (will activate body if needed)
		void MoveKinematic(const BodyID& inBodyID, RVec3Arg inTargetPosition, QuatArg inTargetRotation, float inDeltaTime);

		/// Linear or angular velocity (functions will activate body if needed).
		/// Note that the linear velocity is the velocity of the center of mass, which may not coincide with the position of your object, to correct for this: \f$VelocityCOM = Velocity - AngularVelocity \times ShapeCOM\f$
		void SetLinearAndAngularVelocity(const BodyID& inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity);
		void GetLinearAndAngularVelocity(const BodyID& inBodyID, Vec3& outLinearVelocity, Vec3& outAngularVelocity) const;
		void SetLinearVelocity(const BodyID& inBodyID, Vec3Arg inLinearVelocity);
		Vec3 GetLinearVelocity(const BodyID& inBodyID) const;
		void AddLinearVelocity(const BodyID& inBodyID, Vec3Arg inLinearVelocity); ///< Add velocity to current velocity
		void AddLinearAndAngularVelocity(const BodyID& inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity); ///< Add linear and angular to current velocities
		void SetAngularVelocity(const BodyID& inBodyID, Vec3Arg inAngularVelocity);
		Vec3 GetAngularVelocity(const BodyID& inBodyID) const;
		Vec3 GetPointVelocity(const BodyID& inBodyID, RVec3Arg inPoint) const; ///< Velocity of point inPoint (in world space, e.g. on the surface of the body) of the body

		/// Set the complete motion state of a body.
		/// Note that the linear velocity is the velocity of the center of mass, which may not coincide with the position of your object, to correct for this: \f$VelocityCOM = Velocity - AngularVelocity \times ShapeCOM\f$
		void SetPositionRotationAndVelocity(const BodyID& inBodyID, RVec3Arg inPosition, QuatArg inRotation, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity);

		///@name Add forces to the body
		///@{
		void AddForce(const BodyID& inBodyID, Vec3Arg inForce); ///< See Body::AddForce
		void AddForce(const BodyID& inBodyID, Vec3Arg inForce, RVec3Arg inPoint); ///< Applied at inPoint
		void AddTorque(const BodyID& inBodyID, Vec3Arg inTorque); ///< See Body::AddTorque
		void AddForceAndTorque(const BodyID& inBodyID, Vec3Arg inForce, Vec3Arg inTorque); ///< A combination of Body::AddForce and Body::AddTorque
		///@}

		///@name Add an impulse to the body
		///@{
		void AddImpulse(const BodyID& inBodyID, Vec3Arg inImpulse); ///< Applied at center of mass
		void AddImpulse(const BodyID& inBodyID, Vec3Arg inImpulse, RVec3Arg inPoint); ///< Applied at inPoint
		void AddAngularImpulse(const BodyID& inBodyID, Vec3Arg inAngularImpulse);
		///@}

		///@name Body motion type
		///@{
		void SetMotionType(const BodyID& inBodyID, EMotionType inMotionType, EActivation inActivationMode);
		EMotionType					GetMotionType(const BodyID& inBodyID) const;
		///@}

		///@name Body motion quality
		///@{
		void SetMotionQuality(const BodyID& inBodyID, EMotionQuality inMotionQuality);
		EMotionQuality				GetMotionQuality(const BodyID& inBodyID) const;
		///@}

		/// Get inverse inertia tensor in world space
		Mat44 GetInverseInertia(const BodyID& inBodyID) const;

		///@name Restitution
		///@{
		void SetRestitution(const BodyID& inBodyID, float inRestitution);
		float GetRestitution(const BodyID& inBodyID) const;
		///@}

		///@name Friction
		///@{
		void SetFriction(const BodyID& inBodyID, float inFriction);
		float GetFriction(const BodyID& inBodyID) const;
		///@}

		///@name Gravity factor
		///@{
		void SetGravityFactor(const BodyID& inBodyID, float inGravityFactor);
		float GetGravityFactor(const BodyID& inBodyID) const;
		///@}

		/// Get transform and shape for this body, used to perform collision detection
		TransformedShape			GetTransformedShape(const BodyID& inBodyID) const;

		/// Get the user data for a body
		uint64 GetUserData(const BodyID& inBodyID) const;

		/// Get the material for a particular sub shape
		const PhysicsMaterial* GetMaterial(const BodyID& inBodyID, const SubShapeID& inSubShapeID) const;

		/// Set the Body::EFlags::InvalidateContactCache flag for the specified body. This means that the collision cache is invalid for any body pair involving that body until the next physics step.
		void InvalidateContactCache(const BodyID& inBodyID);
		*/

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
    };
}
