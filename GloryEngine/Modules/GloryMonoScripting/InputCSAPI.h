#pragma once
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class Engine;

	class InputCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);
	};
}
