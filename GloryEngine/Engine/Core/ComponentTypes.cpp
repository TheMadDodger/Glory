#include "ComponentTypes.h"

namespace Glory::ComponentTypes
{
	uint32_t GetComponentHash(const std::string& name)
	{
		return Utils::ECS::ComponentTypes::GetComponentHash(name.find("Glory::") != std::string::npos ? name : "Glory::" + name);
	}
}