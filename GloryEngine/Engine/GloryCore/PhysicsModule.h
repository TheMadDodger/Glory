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

		virtual uint32_t CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const BodyType bodyType) = 0;
		virtual void DestroyPhysicsBody(uint32_t& bodyID) = 0;
		virtual void PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation) = 0;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
    };
}
