#pragma once
#include <Module.h>

namespace Glory::Editor
{
	class IEditorLoopHandler
	{
	public:
		virtual const char* ModuleName() = 0;
		virtual void HandleUpdate(Module* pModule) = 0;
	};
}
