#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class Engine;

	class PhysicsComponentsCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);

	public:
		PhysicsComponentsCSAPI() = delete;
	};
}
