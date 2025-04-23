#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class Engine;

	class LocalizeCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);

	public:
		LocalizeCSAPI() = delete;
	};
}