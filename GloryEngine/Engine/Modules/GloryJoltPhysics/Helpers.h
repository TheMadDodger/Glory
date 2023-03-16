#pragma once
#include <Physics.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>


namespace Glory
{
	JPH::Vec3 ToJPHVec3(const glm::vec3& v);
	JPH::Quat ToJPHQuat(const glm::quat& q);

	glm::vec3 ToVec3(const JPH::Vec3& v);
	glm::quat ToQuat(const JPH::Quat& q);

	JPH::Shape* GetJPHShape(const Shape& shape);
}
