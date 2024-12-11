#pragma once
#include "UUID.h"

#include <vector>
#include <glm/ext/vector_int2.hpp>

namespace Glory
{
	struct Character
	{
		UUID TextureID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		uint32_t Advance;
	};
}
