#pragma once
#include "Shapes.h"

#include <Reflection.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

REFLECTABLE_ENUM_NS(Glory, BodyType, Static, Kinematic, Dynamic);
REFLECTABLE_ENUM_NS(Glory, MotionQuality, Discrete, LinearCast);

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

	enum class AllowedDOFFlag : uint32_t {
		None = 0b000000,
		TranslationX = 1 << 0,
		TranslationY = 1 << 1,
		TranslationZ = 1 << 2,
		RotationX = 1 << 3,
		RotationY = 1 << 4,
		RotationZ = 1 << 5,
		All = TranslationX | TranslationY | TranslationZ | RotationX | RotationY | RotationZ,
	};
}

const std::string Enum<Glory::AllowedDOFFlag>::m_EnumStringValues[] = {
	"None",
	"TranslationX",
	"TranslationY",
	"TranslationZ",
	"RotationX",
	"RotationY",
	"RotationZ",
};

const size_t Enum<Glory::AllowedDOFFlag>::m_NumValues = 3;

bool Enum<Glory::AllowedDOFFlag>::Valid() {
	return true;
};