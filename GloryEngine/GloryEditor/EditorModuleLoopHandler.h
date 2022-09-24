#pragma once
#include <IModuleLoopHandler.h>

namespace Glory::Editor
{
	class EditorModuleLoopHandler : public IModuleLoopHandler
	{
	public:
		virtual bool HandleModuleLoop(Module* pModule) override;
	};
}
