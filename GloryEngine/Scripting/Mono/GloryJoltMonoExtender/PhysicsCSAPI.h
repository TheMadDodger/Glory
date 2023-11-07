#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;

	class PhysicsCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);

	public:
		PhysicsCSAPI() = delete;
	};
}
