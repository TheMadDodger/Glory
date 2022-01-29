#pragma once
#include <glm/glm.hpp>

namespace Glory
{
	struct PointLight
	{
		glm::vec4 position;
		glm::vec4 color;
		uint32_t enabled;
		float intensity;
		float range;
		uint32_t padding;
	};
}