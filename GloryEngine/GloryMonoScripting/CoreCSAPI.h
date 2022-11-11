#pragma once
#include "ScriptExtensions.h"
#include "GloryMono.h"
#include <Debug.h>

namespace Glory
{
	class CoreCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);

	private:
		CoreCSAPI();
		virtual ~CoreCSAPI();
	};
}
