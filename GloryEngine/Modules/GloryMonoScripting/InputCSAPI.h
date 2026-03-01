#pragma once
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class IEngine;

	class InputCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(IEngine* pEngine);
	};
}
