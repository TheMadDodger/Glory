#pragma once
#include <map>
#include <ScriptingExtender.h>
#include "AssemblyBinding.h"

namespace Glory
{
	class MonoLibManager
	{
	public:
		static void LoadLib(MonoDomain* pDomain, const ScriptingLib& lib);
		static void Cleanup();
		static AssemblyBinding* GetAssembly(const std::string& name);

		static MonoObject* InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args);

		static void GetAllClasses();

		static size_t AssemblyCount();
		static void ForEachAssembly(std::function<void(AssemblyBinding*)> callback);

	private:
		MonoLibManager();
		virtual ~MonoLibManager();

	private:
		static std::map<std::string, AssemblyBinding> m_Assemblies;
	};
}