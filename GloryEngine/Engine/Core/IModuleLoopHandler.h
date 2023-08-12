#pragma once
#include "Module.h"

namespace Glory
{
	class IModuleLoopHandler
	{
	public:
		virtual bool HandleModuleLoop(Module* pModule) = 0;
	};
}
