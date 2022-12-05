#pragma once
#include <Glory.h>
#include <ComponentTypes.h>

namespace Glory
{
	class ComponentTypes
	{
	public:
		GLORY_API static size_t GetComponentHash(const std::string& name);
	};
}
