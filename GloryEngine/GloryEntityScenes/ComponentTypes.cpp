#include "ComponentTypes.h"

namespace Glory
{
	size_t ComponentTypes::GetComponentHash(const std::string& name)
	{
		return GloryECS::ComponentTypes::GetComponentHash(name.find("Glory::") != std::string::npos ? name : "Glory::" + name);
	}
}