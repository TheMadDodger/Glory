#pragma once
#include "AssemblyBinding.h"

namespace Glory
{
	class IMonoLibManager
	{
	public:
		virtual void Initialize(AssemblyBinding* pAssembly) = 0;
		virtual void Cleanup() = 0;
	};
}
