#pragma once
#include <PhysicsModule.h>

#include "LayerCollisionFilter.h"
#include "BroadPhaseImpl.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace Glory
{
	struct Shape;

    class JoltPhysicsModule : public PhysicsModule
    {
	public:
		GLORY_API JoltPhysicsModule();
		GLORY_API virtual ~JoltPhysicsModule();

		GLORY_MODULE_VERSION_H;

		/* Body management */
		GLORY_API uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType, const uint16_t layerIndex) override;
		GLORY_API void DestroyPhysicsBody(uint32_t& bodyID) override;
		GLORY_API void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) override;

		/* States */
		GLORY_API void ActivateBody(uint32_t bodyID) override;
		GLORY_API void DeactivateBody(uint32_t bodyID) override;
		GLORY_API bool IsBodyActive(uint32_t bodyID) const override;
		GLORY_API bool IsValidBody(uint32_t bodyID) const override;

		/* Position and rotation */
		GLORY_API void SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType = ActivationType::Activate) override;
		GLORY_API void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate) override;
		GLORY_API void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate) override;
		GLORY_API glm::vec3 GetBodyPosition(uint32_t bodyID) const override;
		GLORY_API glm::vec3 GetBodyCenterOfMassPosition(uint32_t bodyID) const override;
		GLORY_API glm::quat GetBodyRotation(uint32_t bodyID) const override;

		/* Velocities */
		GLORY_API void MoveBodyKinematic(uint32_t bodyID, const glm::vec3& targetPosition, const glm::quat& targetRotation, float deltaTime) override;
		GLORY_API void SetBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;
		GLORY_API void GetBodyLinearAndAngularVelocity(uint32_t bodyID, glm::vec3& linearVelocity, glm::vec3& angularVelocity) const override;
		GLORY_API void SetBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) override;
		GLORY_API glm::vec3 GetBodyLinearVelocity(uint32_t bodyID) const override;
		GLORY_API void AddBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) override;
		GLORY_API void AddBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;
		GLORY_API void SetBodyAngularVelocity(uint32_t bodyID, const glm::vec3& angularVelocity) override;
		GLORY_API glm::vec3 GetBodyAngularVelocity(uint32_t bodyID) const override;
		GLORY_API glm::vec3 GetBodyPointVelocity(uint32_t bodyID, const glm::vec3& point) const override;
		GLORY_API void SetBodyPositionRotationAndVelocity(uint32_t bodyID, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;

		/* Forces */
		GLORY_API void AddBodyForce(uint32_t bodyID, const glm::vec3& force) override;
		GLORY_API void AddBodyForce(uint32_t bodyID, const glm::vec3& force, const glm::vec3& point) override;
		GLORY_API void AddBodyTorque(uint32_t bodyID, const glm::vec3& torque) override;
		GLORY_API void AddBodyForceAndTorque(uint32_t bodyID, const glm::vec3& force, const glm::vec3& torque) override;

		/* Impulses */
		GLORY_API void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse) override;
		GLORY_API void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse, const glm::vec3& point) override;
		GLORY_API void AddBodyAngularImpulse(uint32_t bodyID, const glm::vec3& angularImpulse) override;

		/* Layer */
		GLORY_API void SetBodyObjectLayer(uint32_t bodyID, const uint16_t layerIndex) override;
		GLORY_API const uint16_t GetBodyObjectLayer(uint32_t bodyID) const override;

		GLORY_API BPLayerInterfaceImpl& BPLayerImpl();
		GLORY_API ObjectVsBroadPhaseLayerFilterImpl& BPCollisionFilter();

		void TriggerLateActivationCallback(ActivationCallback callbackType, uint32_t bodyID);

	private:
		virtual void LoadSettings(ModuleSettings& settings) override;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;

	private:
		JPH::TempAllocatorImpl* m_pJPHTempAllocator;
		JPH::JobSystem* m_pJPHJobSystem;
		JPH::PhysicsSystem* m_pJPHPhysicsSystem;

		LayerCollisionFilter m_CollisionFilter;
		BPLayerInterfaceImpl m_BPLayerImpl;
		ObjectVsBroadPhaseLayerFilterImpl m_ObjectVSBroadPhase;

		MyBodyActivationListener m_BodyActivationListener;
		MyContactListener m_ContactListener;

		std::map<ActivationCallback, std::vector<uint32_t>> m_LateCallbacks;
    };
}
