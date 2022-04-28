#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <ScriptBinding.h>

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
		static void CreateBindings(std::vector<ScriptBinding>& bindings);
	};
}
