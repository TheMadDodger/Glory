#pragma once
#include <string>
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	class IScriptExtender
	{
	public:
		virtual std::string Language() = 0;
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) = 0;
		virtual void GetLibs(std::vector<ScriptingLib>& libs) = 0;
	};
}
