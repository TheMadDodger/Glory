#include "MonoLibManager.h"

namespace Glory
{
	std::map<std::string, AssemblyBinding> MonoLibManager::m_Assemblies;

	void MonoLibManager::LoadLib(MonoDomain* pDomain, const ScriptingLib& lib)
	{
		std::string name = lib.LibraryName();
		size_t index = m_Assemblies.size();
		m_Assemblies.emplace(name, AssemblyBinding(pDomain, lib.LibraryName()));
		m_Assemblies[name].Initialize();
	}

	void MonoLibManager::Cleanup()
	{
		for (auto it = m_Assemblies.begin(); it != m_Assemblies.end(); it++)
		{
			it->second.Destroy();
		}
		m_Assemblies.clear();
	}

	AssemblyBinding* MonoLibManager::GetAssembly(const std::string& name)
	{
		if (m_Assemblies.find(name) == m_Assemblies.end()) return nullptr;
		return &m_Assemblies[name];
	}

	MonoObject* MonoLibManager::InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args)
	{
		return mono_runtime_invoke(pMethod, pObject, args, pExceptionObject);
	}

	void MonoLibManager::GetAllClasses()
	{
		//MonoType* pType = mono_class_get_type();
		//mono_class_is_subclass_of
	}

	size_t MonoLibManager::AssemblyCount()
	{
		return m_Assemblies.size();
	}

	void MonoLibManager::ForEachAssembly(std::function<void(AssemblyBinding*)> callback)
	{
		for (auto it : m_Assemblies)
		{
			callback(&it.second);
		}
	}

	MonoLibManager::MonoLibManager()
	{
	}

	MonoLibManager::~MonoLibManager()
	{
	}
}
