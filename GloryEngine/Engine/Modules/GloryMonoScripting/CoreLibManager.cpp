#include "CoreLibManager.h"
#include "MonoAssetManager.h"

namespace Glory
{
	CoreLibManager::CoreLibManager() : m_pAssembly(nullptr)
	{
	}

	AssemblyBinding* CoreLibManager::GetAssemblyBinding() const
	{
		return m_pAssembly;
	}

	void CoreLibManager::Initialize(AssemblyBinding* pAssembly)
	{
		m_pAssembly = pAssembly;
		MonoAssetManager::Initialize(m_pAssembly->GetMonoImage());
	}

	void CoreLibManager::Cleanup()
	{
	}
}
