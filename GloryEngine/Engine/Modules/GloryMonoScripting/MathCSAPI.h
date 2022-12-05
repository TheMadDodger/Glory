#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class MathCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
	};
}
