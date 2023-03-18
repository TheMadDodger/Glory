#pragma once
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class PhysicsCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);

	public:
		PhysicsCSAPI() = delete;
	};
}
