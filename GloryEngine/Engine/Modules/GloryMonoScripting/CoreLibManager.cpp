#include "CoreLibManager.h"
#include "MonoAssetManager.h"
#include "MonoManager.h"
#include "ScriptingMethodsHelper.h"
#include "Assembly.h"

namespace Glory
{
	CoreLibManager::CoreLibManager(MonoManager* pMonoManager) : m_pMonoManager(pMonoManager), m_pAssembly(nullptr)
	{
	}

	Assembly* CoreLibManager::GetAssemblyBinding() const
	{
		return m_pAssembly;
	}

	void CoreLibManager::Initialize(Assembly* pAssembly)
	{
		m_pAssembly = pAssembly;
		MonoAssetManager::Initialize(m_pAssembly->GetMonoImage());
		m_pMonoManager->GetMethodsHelper()->Initialize(m_pAssembly->GetMonoImage());
	}

	void CoreLibManager::Cleanup()
	{
	}
}
