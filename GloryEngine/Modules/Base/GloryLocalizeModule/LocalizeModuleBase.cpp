#include "LocalizeModuleBase.h"

namespace Glory
{
	LocalizeModuleBase::LocalizeModuleBase() {}
	LocalizeModuleBase::~LocalizeModuleBase() {}

	const std::type_info& LocalizeModuleBase::GetBaseModuleType()
	{
		return typeid(LocalizeModuleBase);
	}
}