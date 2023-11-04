#include "ComponentTypes.h"

namespace Glory
{
	uint32_t ComponentTypes::GetComponentHash(const std::string& name)
	{
		return Utils::ECS::ComponentTypes::GetComponentHash(name.find("Glory::") != std::string::npos ? name : "Glory::" + name);
	}
}