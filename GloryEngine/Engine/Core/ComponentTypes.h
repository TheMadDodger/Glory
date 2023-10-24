#pragma once
#include <GloryECS/ComponentTypes.h>

namespace Glory
{
	struct ComponentTypes
	{
		static uint32_t GetComponentHash(const std::string& name);
	};
}
