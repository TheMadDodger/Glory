#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class IEngine;

	class AudioCSAPI
	{
	public:
		static void GetInternallCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(IEngine* pEngine);
	};
}
