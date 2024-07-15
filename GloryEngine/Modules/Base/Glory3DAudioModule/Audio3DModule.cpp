#include "Audio3DModule.h"

namespace Glory
{
	Audio3DModule::Audio3DModule()
	{
	}

	Audio3DModule::~Audio3DModule()
	{
	}

	const std::type_info& Audio3DModule::GetBaseModuleType()
	{
		return typeid(Audio3DModule);
	}
}