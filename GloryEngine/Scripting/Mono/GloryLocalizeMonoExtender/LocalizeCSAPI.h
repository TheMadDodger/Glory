#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class IEngine;

	class LocalizeCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(IEngine* pEngine);

	public:
		LocalizeCSAPI() = delete;
	};
}