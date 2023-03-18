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
}
