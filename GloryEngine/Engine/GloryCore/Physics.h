#pragma once
#include "Shapes.h"

#include <Reflection.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

REFLECTABLE_ENUM_NS(Glory, BodyType, Static, Kinematic, Dynamic);

namespace Glory
{
	enum class ActivationType
	{
		/* Activate the body */
		Activate,
		/* Leave activation state as is */
		DontActivate
	};

	struct Ray
	{
		glm::vec3 m_Origin;
		glm::vec3 m_Direction;

		glm::vec3 GetPointOnRay(float distance) const
		{
			return m_Origin + m_Direction * distance;
		}
	};

	struct RayCastHit
	{
		float m_Distance;
		uint32_t m_BodyID;
		uint32_t m_SubShapeID;
		glm::vec3 m_Pos;
	};

	struct RayCastResult
	{
		std::vector<RayCastHit> m_Hits;
	};
}
