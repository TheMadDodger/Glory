#include "MonoManager.h"
#include "MonoLibManager.h"
#include "MonoObjectManager.h"

namespace Glory
{
	MonoDomain* MonoManager::m_pMainDomain = nullptr;
	MonoDomain* MonoManager::m_pDomain = nullptr;

	void MonoManager::Initialize(const std::string& assemblyDir, const std::string& configDir)
	{
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());
		m_pMainDomain = mono_jit_init_version("GloryMain", "v4.0.30319");
		m_pDomain = mono_domain_create_appdomain("GloryMain-1", NULL);
		mono_domain_set(m_pDomain, false);
	}

	void MonoManager::Cleanup()
	{
		mono_domain_unload(m_pDomain);
		mono_domain_finalize(m_pDomain, 2000);

		MonoLibManager::Cleanup();
		if (m_pMainDomain) mono_jit_cleanup(m_pMainDomain);
		m_pDomain = nullptr;
	}

	MonoDomain* MonoManager::GetDomain()
	{
		return m_pDomain;
	}

	void MonoManager::LoadLib(const ScriptingLib& lib)
	{
		MonoLibManager::LoadLib(m_pDomain, lib);
	}

	void MonoManager::Reload()
	{
		// Create new domain
		MonoDomain* pNewDomain = mono_domain_create_appdomain("GloryMain-1", NULL);
		mono_domain_set(pNewDomain, false);

		// Unload domain
		mono_domain_unload(m_pDomain);
		mono_domain_finalize(m_pDomain, 2000);

		// Load assemblies into this domain
		MonoObjectManager::Cleanup();
		MonoLibManager::ReloadAll(pNewDomain);

		m_pDomain = pNewDomain;
	}

	MonoManager::MonoManager()
	{
	}

	MonoManager::~MonoManager()
	{
	}
}
