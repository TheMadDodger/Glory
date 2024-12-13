#pragma once
#include "UUID.h"

#include <vector>
#include <glm/ext/vector_int2.hpp>

namespace Glory
{
	struct GlyphData
	{
		uint64_t Code;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		uint32_t Advance;
	};
}
