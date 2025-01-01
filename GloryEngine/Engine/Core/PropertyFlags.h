#pragma once
#include <glm/glm.hpp>

namespace Glory
{
	enum PropertyFlags : uint32_t
	{
		None = 1 << 0,
		Color = 1 << 1,
		HDR = 1 << 2,
		AreaText = 1 << 3,
	};
}
