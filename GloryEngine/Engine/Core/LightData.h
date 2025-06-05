#pragma once
#include <glm/glm.hpp>
#include <Reflection.h>

REFLECTABLE_ENUM_NS_SIZE(Glory, LightType, uint32_t, None, Sun, Point, Spot);

namespace Glory
{
	struct LightData
	{
		glm::vec3 position;
		LightType type;
		glm::vec4 direction;
		glm::vec4 color;
		glm::vec4 data;
		uint32_t shadowsEnabled;
		float shadowBias;
		float padding1;
		float padding2;
		glm::vec4 shadowCoords;
	};
}
