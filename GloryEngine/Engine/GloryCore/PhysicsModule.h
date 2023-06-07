#pragma once
#include "Module.h"
#include "Physics.h"
#include "CharacterManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Glory
{
	struct Layer;

	enum class ContactCallback
	{
		Added,
		Persisted,
		Removed,

		Count
	};

	enum class ActivationCallback
	{
		Activated,
		Deactivated,

		Count
	};

    class PhysicsModule : public Module
    {
	public:
		PhysicsModule();
		virtual ~PhysicsModule();

		virtual const std::type_info& GetModuleType() override;

		/* Body management */
		virtual uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType, const uint16_t layerIndex = 0) = 0;
		virtual void DestroyPhysicsBody(uint32_t& bodyID) = 0;
		virtual void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) = 0;
		
		/* States */
		virtual void ActivateBody(uint32_t bodyID) = 0;
		virtual void DeactivateBody(uint32_t bodyID) = 0;
		virtual bool IsBodyActive(uint32_t bodyID) const = 0;
		virtual bool IsValidBody(uint32_t bodyID) const = 0;

		/* Position and rotation */
		virtual void SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate) = 0;
		virtual glm::vec3 GetBodyPosition(uint32_t bodyID) const = 0;
		virtual glm::vec3 GetBodyCenterOfMassPosition(uint32_t bodyID) const = 0;
		virtual glm::quat GetBodyRotation(uint32_t bodyID) const = 0;
		//virtual glm::mat4 GetBodyWorldTransform(uint32_t bodyID) const = 0;
		//virtual glm::mat4 GetBodyCenterOfMassTransform(uint32_t bodyID) const = 0;

		/* Velocities */
		virtual void MoveBodyKinematic(uint32_t bodyID, const glm::vec3& targetPosition, const glm::quat& targetRotation, float deltaTime) = 0;
		virtual void SetBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) = 0;
		virtual void GetBodyLinearAndAngularVelocity(uint32_t bodyID, glm::vec3& linearVelocity, glm::vec3& angularVelocity) const = 0;
		virtual void SetBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) = 0;
		virtual glm::vec3 GetBodyLinearVelocity(uint32_t bodyID) const = 0;
		virtual void AddBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) = 0;
		virtual void AddBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) = 0;
		virtual void SetBodyAngularVelocity(uint32_t bodyID, const glm::vec3& angularVelocity) = 0;
		virtual glm::vec3 GetBodyAngularVelocity(uint32_t bodyID) const = 0;
		virtual glm::vec3 GetBodyPointVelocity(uint32_t bodyID, const glm::vec3& point) const = 0;
		virtual void SetBodyPositionRotationAndVelocity(uint32_t bodyID, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) = 0;

		/* Forces */
		virtual void AddBodyForce(uint32_t bodyID, const glm::vec3& force) = 0;
		virtual void AddBodyForce(uint32_t bodyID, const glm::vec3& force, const glm::vec3& point) = 0;
		virtual void AddBodyTorque(uint32_t bodyID, const glm::vec3& torque) = 0;
		virtual void AddBodyForceAndTorque(uint32_t bodyID, const glm::vec3& force, const glm::vec3& torque) = 0;

		/* Impulses */
		virtual void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse) = 0;
		virtual void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse, const glm::vec3& point) = 0;
		virtual void AddBodyAngularImpulse(uint32_t bodyID, const glm::vec3& angularImpulse) = 0;

		/* Layer */
		virtual void SetBodyObjectLayer(uint32_t bodyID, const uint16_t layerIndex) = 0;
		virtual const uint16_t GetBodyObjectLayer(uint32_t bodyID) const = 0;

		/* Raycast */
		virtual bool CastRay(const glm::uvec3& origin, const glm::vec3& direction, RayCastResult& result) const = 0;

		GLORY_API void SetCollisionMatrix(std::vector<std::vector<bool>>&& matrix);
		GLORY_API bool ShouldCollide(uint16_t layer1, uint16_t layer2) const;

		GLORY_API void RegisterContactCallback(ContactCallback callbackType, std::function<void(uint32_t, uint32_t)> callback);
		GLORY_API void RegisterActivationCallback(ActivationCallback callbackType, std::function<void(uint32_t)> callback);

		GLORY_API void TriggerContactCallback(ContactCallback callbackType, uint32_t bodyID1, uint32_t bodyID2);
		GLORY_API void TriggerActivationCallback(ActivationCallback callbackType, uint32_t bodyID);

		virtual CharacterManager* GetCharacterManager() = 0;

		/* Gravity */
		virtual void SetGravity(const glm::vec3& gravity) = 0;
		virtual const glm::vec3 GetGravity() const = 0;
		/*

		/// Create a two body constraint
		TwoBodyConstraint* CreateConstraint(const TwoBodyConstraintSettings* inSettings, const BodyID& inBodyID1, const BodyID& inBodyID2);

		/// Activate non-static bodies attached to a constraint
		void ActivateConstraint(const TwoBodyConstraint* inConstraint);

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

		bool ShouldCollidePass(uint16_t layer1, uint16_t layer2) const;

	private:
		std::vector<std::vector<bool>> m_CollisionMatrix;
		std::map<ContactCallback, std::vector<std::function<void(uint32_t, uint32_t)>>> m_ContactCallbacks;
		std::map<ActivationCallback, std::vector<std::function<void(uint32_t)>>> m_ActivationCallbacks;
    };
}
