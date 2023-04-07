#include "Helpers.h"

namespace Glory
{
	JPH::Vec3 ToJPHVec3(const glm::vec3& v)
	{
		return JPH::Vec3(v.x, v.y, v.z);
	}

	JPH::Quat ToJPHQuat(const glm::quat& q)
	{
		return JPH::Quat(q.x, q.y, q.z, q.w);
	}

	glm::vec3 ToVec3(const JPH::Vec3& v)
	{
		return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
	}

	glm::quat ToQuat(const JPH::Quat& q)
	{
		return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
	}

	glm::vec4 ToVec3(const JPH::Color& v)
	{
		return glm::vec4(v.r, v.g, v.b, v.a);
	}
}
