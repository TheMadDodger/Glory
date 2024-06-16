#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class Engine;

	class AudioCSAPI
	{
	public:
		static void GetInternallCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);
	};
}
