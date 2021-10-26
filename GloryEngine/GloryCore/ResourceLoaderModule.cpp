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

	ImportSettings::ImportSettings() : m_Extension("") {}
	ImportSettings::ImportSettings(const std::string& extension) : m_Extension(extension) {}
}