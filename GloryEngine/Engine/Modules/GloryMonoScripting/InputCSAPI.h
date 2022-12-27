#pragma once
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class InputCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
	};
}
