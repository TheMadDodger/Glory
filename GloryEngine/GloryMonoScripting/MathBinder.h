#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	struct Vec2Wrapper
	{
		float x;
		float y;
	};

	class MathBinder
	{
	public:
		static void CreateBindings(std::vector<InternalCall>& internalCalls);
	};
}
