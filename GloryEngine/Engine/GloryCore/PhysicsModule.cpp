#include "PhysicsModule.h"

namespace Glory
{
	PhysicsModule::PhysicsModule()
	{
	}

	PhysicsModule::~PhysicsModule()
	{
	}

	const std::type_info& PhysicsModule::GetModuleType()
	{
		return typeid(PhysicsModule);
	}
}
