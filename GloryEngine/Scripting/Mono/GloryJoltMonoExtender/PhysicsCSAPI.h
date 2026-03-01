#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class IEngine;

	class PhysicsCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(IEngine* pEngine);

	public:
		PhysicsCSAPI() = delete;
	};
}
