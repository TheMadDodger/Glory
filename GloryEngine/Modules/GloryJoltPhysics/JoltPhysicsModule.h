#pragma once
#include <Physics.h>

#include <PhysicsModule.h>
#include <glm/fwd.hpp>

namespace JPH
{
	class TempAllocatorImpl;
	class JobSystem;
	class PhysicsSystem;
}

namespace Glory
{
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

	struct Shape;
	struct Layer;
	struct LayerMask;
	struct PhysicsSimulationSettings;

	class ObjectVsBroadPhaseLayerFilterImpl;
	class BPLayerInterfaceImpl;
	class JoltCharacterManager;
	class JoltShapeManager;

    class JoltPhysicsModule : public PhysicsModule
    {
	public:
		GLORY_API JoltPhysicsModule();
		GLORY_API virtual ~JoltPhysicsModule();

		GLORY_MODULE_VERSION_H(0,6,0);

		/* Body management */
		GLORY_API uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType, const uint16_t layerIndex, const PhysicsSimulationSettings& settings);
		GLORY_API void SetBodyUserData(uint32_t bodyID, uint64_t userData);
		GLORY_API uint64_t GetBodyUserData(uint32_t bodyID);
		GLORY_API void DestroyPhysicsBody(uint32_t& bodyID);
		GLORY_API void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation);

		/* States */
		GLORY_API void ActivateBody(uint32_t bodyID);
		GLORY_API void DeactivateBody(uint32_t bodyID);
		GLORY_API bool IsBodyActive(uint32_t bodyID) const;
		GLORY_API bool IsValidBody(uint32_t bodyID) const;
		GLORY_API void SetBodyType(uint32_t bodyID, BodyType bodyType, const ActivationType activationType);

		/* Position and rotation */
		GLORY_API void SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType = ActivationType::Activate);
		GLORY_API void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate);
		GLORY_API void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate);
		GLORY_API glm::vec3 GetBodyPosition(uint32_t bodyID) const;
		GLORY_API glm::vec3 GetBodyCenterOfMassPosition(uint32_t bodyID) const;
		GLORY_API glm::quat GetBodyRotation(uint32_t bodyID) const;

		/* Velocities */
		GLORY_API void MoveBodyKinematic(uint32_t bodyID, const glm::vec3& targetPosition, const glm::quat& targetRotation, float deltaTime);
		GLORY_API void SetBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity);
		GLORY_API void GetBodyLinearAndAngularVelocity(uint32_t bodyID, glm::vec3& linearVelocity, glm::vec3& angularVelocity) const;
		GLORY_API void SetBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity);
		GLORY_API glm::vec3 GetBodyLinearVelocity(uint32_t bodyID) const;
		GLORY_API void AddBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity);
		GLORY_API void AddBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity);
		GLORY_API void SetBodyAngularVelocity(uint32_t bodyID, const glm::vec3& angularVelocity);
		GLORY_API glm::vec3 GetBodyAngularVelocity(uint32_t bodyID) const;
		GLORY_API glm::vec3 GetBodyPointVelocity(uint32_t bodyID, const glm::vec3& point) const;
		GLORY_API void SetBodyPositionRotationAndVelocity(uint32_t bodyID, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity);

		/* Forces */
		GLORY_API void AddBodyForce(uint32_t bodyID, const glm::vec3& force);
		GLORY_API void AddBodyForce(uint32_t bodyID, const glm::vec3& force, const glm::vec3& point);
		GLORY_API void AddBodyTorque(uint32_t bodyID, const glm::vec3& torque);
		GLORY_API void AddBodyForceAndTorque(uint32_t bodyID, const glm::vec3& force, const glm::vec3& torque);

		/* Impulses */
		GLORY_API void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse);
		GLORY_API void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse, const glm::vec3& point);
		GLORY_API void AddBodyAngularImpulse(uint32_t bodyID, const glm::vec3& angularImpulse);

		/* Layer */
		GLORY_API void SetBodyObjectLayer(uint32_t bodyID, const uint16_t layerIndex);
		GLORY_API const uint16_t GetBodyObjectLayer(uint32_t bodyID) const;

		/* RayCast */
		GLORY_API bool CastRay(const Ray& ray, RayCastResult& result, float maxDistance, const LayerMask& layerMask, const std::vector<uint32_t>& ignoreBodyIDs) const;

		GLORY_API BPLayerInterfaceImpl& BPLayerImpl();
		GLORY_API ObjectVsBroadPhaseLayerFilterImpl& BPCollisionFilter();

		GLORY_API void TriggerLateActivationCallback(ActivationCallback callbackType, uint32_t bodyID);
		GLORY_API void TriggerLateContactCallback(ContactCallback callbackType, uint32_t body1ID, uint32_t body2ID);

		GLORY_API JoltCharacterManager* GetCharacterManager();
		GLORY_API JoltShapeManager* GetShapeManager();

		/* Gravity */
		GLORY_API void SetGravity(const glm::vec3& gravity);
		GLORY_API const glm::vec3 GetGravity() const;

		GLORY_API void CleanupPhysics();
		GLORY_API void SetupPhysics();

		GLORY_API void SetCollisionMatrix(std::vector<std::vector<bool>>&& matrix);
		GLORY_API bool ShouldCollide(uint16_t layer1, uint16_t layer2) const;

		GLORY_API void RegisterContactCallback(ContactCallback callbackType, std::function<void(JoltPhysicsModule*, uint32_t, uint32_t)> callback);
		GLORY_API void RegisterActivationCallback(ActivationCallback callbackType, std::function<void(JoltPhysicsModule*, uint32_t)> callback);

		GLORY_API void TriggerContactCallback(ContactCallback callbackType, uint32_t bodyID1, uint32_t bodyID2);
		GLORY_API void TriggerActivationCallback(ActivationCallback callbackType, uint32_t bodyID);

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

	private:
		virtual void LoadSettings(ModuleSettings& settings) override;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;
		virtual void Draw() override;

		virtual void OnProcessData() override;

		virtual const std::type_info& GetModuleType() override;

		bool ShouldCollidePass(uint16_t layer1, uint16_t layer2) const;

	private:
		JPH::TempAllocatorImpl* m_pJPHTempAllocator;
		JPH::JobSystem* m_pJPHJobSystem;
		JPH::PhysicsSystem* m_pJPHPhysicsSystem;

		std::map<ActivationCallback, std::vector<uint32_t>> m_LateActivationCallbacks;
		std::map<ContactCallback, std::vector<std::pair<uint32_t, uint32_t>>> m_LateContactCallbacks;

		std::vector<std::vector<bool>> m_CollisionMatrix;
		std::map<ContactCallback, std::vector<std::function<void(JoltPhysicsModule*, uint32_t, uint32_t)>>> m_ContactCallbacks;
		std::map<ActivationCallback, std::vector<std::function<void(JoltPhysicsModule*, uint32_t)>>> m_ActivationCallbacks;
    };
}