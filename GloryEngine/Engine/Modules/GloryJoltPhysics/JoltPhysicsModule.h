#pragma once
#include <PhysicsModule.h>

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

		uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType) override;
		void DestroyPhysicsBody(uint32_t& bodyID) override;
		void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) override;
		void SetBodyPosition(uint32_t bodyID, const glm::vec3& postion, const ActivationType activationType = ActivationType::Activate) override;
		void SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType = ActivationType::Activate) override;
		void SetBodyScale(uint32_t bodyID, const glm::vec3& scale, const ActivationType activationType = ActivationType::Activate) override;
		void ActivateBody(uint32_t bodyID) override;
		void DeactivateBody(uint32_t bodyID) override;
		bool IsBodyActive(uint32_t bodyID) const override;
		bool IsValidBody(uint32_t bodyID) const override;

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
    };
}
