#include "MonoManager.h"
#include "MonoLibManager.h"

namespace Glory
{
	MonoDomain* MonoManager::m_pDomain = nullptr;

	void MonoManager::Initialize(const std::string& assemblyDir, const std::string& configDir)
	{
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());
		m_pDomain = mono_jit_init_version("GloryMain", "v4.0.30319");
	}

	void MonoManager::Cleanup()
	{
		MonoLibManager::Cleanup();
		if (m_pDomain) mono_jit_cleanup(m_pDomain);
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

	MonoManager::MonoManager()
	{
	}

	MonoManager::~MonoManager()
	{
	}
}
