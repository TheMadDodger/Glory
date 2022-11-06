#pragma once
#include <Module.h>

namespace Glory::Editor
{
	class IPlayModeHandler
	{
	public:
		virtual const char* ModuleName() = 0;
		virtual void HandleStart(Module* pModule) = 0;
		virtual void HandleStop(Module* pModule) = 0;
		virtual void HandleUpdate(Module* pModule) = 0;
	};
}
