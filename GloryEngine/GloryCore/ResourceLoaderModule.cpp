#include "ResourceLoaderModule.h"

namespace Glory
{
	LoaderModule::LoaderModule()
	{
	}

	LoaderModule::~LoaderModule()
	{
	}

	bool LoaderModule::HasPriority()
	{
		return true;
	}
}