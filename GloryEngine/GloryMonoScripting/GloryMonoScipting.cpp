#include "GloryMonoScipting.h"
#include <mono/metadata/assembly.h>
#include <Game.h>

namespace Glory
{
	GloryMonoScipting::GloryMonoScipting() : m_pDomain(nullptr)
	{
	}

	GloryMonoScipting::~GloryMonoScipting()
	{
		m_Assemblies.clear();
		m_pDomain = nullptr;
	}

	void GloryMonoScipting::Initialize()
	{
		//if (Game::GetGame().GetApplicationType() == ApplicationType::AT_Editor) return;
		mono_set_dirs(".", "");
		InitializeDomainAndMainAssembly();
	}
	
	void GloryMonoScipting::SetMonoDirs(const std::string& assemblyDir, const std::string& configDir)
	{
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());
	}

	void GloryMonoScipting::InitializeDomainAndMainAssembly()
	{
		m_pDomain = mono_jit_init_version("GloryMain", "v4.0.30319");
		//m_Assemblies.push_back(AssemblyBinding(m_pDomain, "./GloryCS.dll"));
		m_Assemblies.push_back(AssemblyBinding(m_pDomain, "C:/Users/Sparta/Documents/Glory/GloryEngine/GloryCS/Debug/Win32/GloryCS.dll"));
	}

	void GloryMonoScipting::PostInitialize()
	{
		for (size_t i = 0; i < m_Assemblies.size(); i++)
		{
			m_Assemblies[i].Initialize();
		}
	}

	void GloryMonoScipting::Cleanup()
	{
		for (size_t i = 0; i < m_Assemblies.size(); i++)
		{
			m_Assemblies[i].Destroy();
		}

		if (m_pDomain) mono_jit_cleanup(m_pDomain);
	}
}
