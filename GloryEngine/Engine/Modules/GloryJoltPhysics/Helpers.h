#pragma once
#include <Jolt/Jolt.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Glory
{
	JPH::Vec3 ToJPHVec3(const glm::vec3& v);
	JPH::Quat ToJPHQuat(const glm::quat& q);

	glm::vec3 ToVec3(const JPH::Vec3& v);
	glm::quat ToQuat(const JPH::Quat& q);
}
