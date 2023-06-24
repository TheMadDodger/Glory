#include "AssemblyDomain.h"
#include "Assembly.h"
#include "MonoScriptObjectManager.h"

namespace Glory
{
	AssemblyDomain::AssemblyDomain(const std::string& name, MonoDomain* pDomain)
		: m_Name(name), m_pMonoDomain(pDomain), m_pScriptObjectManager(new MonoScriptObjectManager()) {}

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
		m_Assemblies.emplace(name, Assembly{ this });
		if (lib.IsMainLib()) m_MainAssemblyName = lib.LibraryName();
		m_Assemblies.at(name).Load(lib, pLibManager);
	}

	void AssemblyDomain::Reload(const std::string& name)
	{
		auto& itor = m_Assemblies.find(name);
		if (itor == m_Assemblies.end()) return;
		Assembly& assembly = itor->second;
		assembly.Unload();
	}

	void AssemblyDomain::Unload(bool isReloading)
	{
		m_pScriptObjectManager->Cleanup();
		for (auto it = m_Assemblies.begin(); it != m_Assemblies.end(); it++)
		{
			it->second.Unload(isReloading);
		}
		m_Assemblies.clear();
	}

	Assembly* AssemblyDomain::GetAssembly(const std::string& name)
	{
		if (m_Assemblies.find(name) == m_Assemblies.end()) return nullptr;
		return &m_Assemblies.at(name);
	}

	Assembly* AssemblyDomain::GetMainAssembly()
	{
		return &m_Assemblies.at(m_MainAssemblyName);
	}

	const std::string& AssemblyDomain::GetMainAssemblyName()
	{
		return m_MainAssemblyName;
	}

	MonoScriptObjectManager* AssemblyDomain::ScriptObjectManager()
	{
		return m_pScriptObjectManager;
	}

	MonoObject* AssemblyDomain::InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args)
	{
		return mono_runtime_invoke(pMethod, pObject, args, pExceptionObject);
	}

	size_t AssemblyDomain::AssemblyCount()
	{
		return m_Assemblies.size();
	}

	void AssemblyDomain::ForEachAssembly(std::function<void(Assembly*)> callback)
	{
		for (auto it : m_Assemblies)
		{
			callback(&it.second);
		}
	}

	bool AssemblyDomain::SetCurrentDomain(bool force)
	{
		return mono_domain_set(m_pMonoDomain, force);
	}
}
