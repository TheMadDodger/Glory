#pragma once
#include <PhysicsModule.h>

#include "LayerCollisionFilter.h"
#include "BroadphaseLayerImpl.h"

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
		uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType, const uint16_t layerIndex) override;
		void DestroyPhysicsBody(uint32_t& bodyID) override;
		void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) override;

		/* States */
		void ActivateBody(uint32_t bodyID) override;
		void DeactivateBody(uint32_t bodyID) override;
		bool IsBodyActive(uint32_t bodyID) const override;
		bool IsValidBody(uint32_t bodyID) const override;

		/* Position and rotation */
		void SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType = ActivationType::Activate) override;
		void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate) override;
		void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate) override;
		glm::vec3 GetBodyPosition(uint32_t bodyID) const override;
		glm::vec3 GetBodyCenterOfMassPosition(uint32_t bodyID) const override;
		glm::quat GetBodyRotation(uint32_t bodyID) const override;

		/* Velocities */
		void MoveBodyKinematic(uint32_t bodyID, const glm::vec3& targetPosition, const glm::quat& targetRotation, float deltaTime) override;
		void SetBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;
		void GetBodyLinearAndAngularVelocity(uint32_t bodyID, glm::vec3& linearVelocity, glm::vec3& angularVelocity) const override;
		void SetBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) override;
		glm::vec3 GetBodyLinearVelocity(uint32_t bodyID) const override;
		void AddBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity) override;
		void AddBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;
		void SetBodyAngularVelocity(uint32_t bodyID, const glm::vec3& angularVelocity) override;
		glm::vec3 GetBodyAngularVelocity(uint32_t bodyID) const override;
		glm::vec3 GetBodyPointVelocity(uint32_t bodyID, const glm::vec3& point) const override;
		void SetBodyPositionRotationAndVelocity(uint32_t bodyID, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity) override;

		/* Forces */
		void AddBodyForce(uint32_t bodyID, const glm::vec3& force) override;
		void AddBodyForce(uint32_t bodyID, const glm::vec3& force, const glm::vec3& point) override;
		void AddBodyTorque(uint32_t bodyID, const glm::vec3& torque) override;
		void AddBodyForceAndTorque(uint32_t bodyID, const glm::vec3& force, const glm::vec3& torque) override;

		/* Impulses */
		void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse) override;
		void AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse, const glm::vec3& point) override;
		void AddBodyAngularImpulse(uint32_t bodyID, const glm::vec3& angularImpulse) override;

		/* Layer */
		virtual void SetBodyObjectLayer(uint32_t bodyID, const uint16_t layerIndex) override;
		virtual const uint16_t GetBodyObjectLayer(uint32_t bodyID) const override;

		BPLayerInterfaceImpl& BPLayerImpl();

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
    };
}
