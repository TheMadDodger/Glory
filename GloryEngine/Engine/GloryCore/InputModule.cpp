#include "InputModule.h"

namespace Glory
{
	InputModule::InputModule()
	{
	}

	InputModule::~InputModule()
	{
	}

	const std::type_info& InputModule::GetModuleType()
	{
		return typeid(InputModule);
	}
}
