#include "AssemblyDomain.h"
#include "AssemblyBinding.h"

namespace Glory
{
	AssemblyDomain::AssemblyDomain(const std::string& name, MonoDomain* pDomain)
		: m_Name(name), m_pMonoDomain(pDomain) {}

	AssemblyDomain::~AssemblyDomain()
	{
		m_pMonoDomain = nullptr;
		m_Assemblies.clear();
	}

	void AssemblyDomain::LoadLib(const ScriptingLib& lib)
	{
		const std::string& name = lib.LibraryName();
		const void* data = lib.Data();
		IMonoLibManager* pLibManager = data ? (IMonoLibManager*)data : nullptr;
		m_Assemblies.emplace(name, AssemblyBinding{ lib, pLibManager });
		if (lib.IsMainLib()) m_MainAssemblyName = lib.LibraryName();
		m_Assemblies.at(name).Initialize(m_pMonoDomain);
	}

	void AssemblyDomain::ReloadAll()
	{
		for (auto it = m_Assemblies.begin(); it != m_Assemblies.end(); it++)
		{
			it->second.Destroy();
		}

		for (auto it = m_Assemblies.begin(); it != m_Assemblies.end(); it++)
		{
			it->second.Initialize(m_pMonoDomain);
		}
	}

	void AssemblyDomain::Cleanup()
	{
		for (auto it = m_Assemblies.begin(); it != m_Assemblies.end(); it++)
		{
			it->second.Destroy();
		}
		m_Assemblies.clear();
	}

	AssemblyBinding* AssemblyDomain::GetAssembly(const std::string& name)
	{
		if (m_Assemblies.find(name) == m_Assemblies.end()) return nullptr;
		return &m_Assemblies.at(name);
	}

	AssemblyBinding* AssemblyDomain::GetMainAssembly()
	{
		return &m_Assemblies.at(m_MainAssemblyName);
	}

	const std::string& AssemblyDomain::GetMainAssemblyName()
	{
		return m_MainAssemblyName;
	}

	MonoObject* AssemblyDomain::InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args)
	{
		return mono_runtime_invoke(pMethod, pObject, args, pExceptionObject);
	}

	size_t AssemblyDomain::AssemblyCount()
	{
		return m_Assemblies.size();
	}

	void AssemblyDomain::ForEachAssembly(std::function<void(AssemblyBinding*)> callback)
	{
		for (auto it : m_Assemblies)
		{
			callback(&it.second);
		}
	}
}
