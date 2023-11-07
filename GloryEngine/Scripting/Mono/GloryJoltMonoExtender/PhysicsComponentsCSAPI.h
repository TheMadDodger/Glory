#pragma once

namespace Glory
{
	class InternalCall;

	class PhysicsComponentsCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);

	public:
		PhysicsComponentsCSAPI() = delete;
	};
}
