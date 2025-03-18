#pragma once
#include <glm/glm.hpp>
#include <Reflection.h>

REFLECTABLE_ENUM_NS(Glory, LightType, None, Sun, Point, Spot);

namespace Glory
{
	struct LightData
	{
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 color;
		glm::vec4 data;
	};
}
