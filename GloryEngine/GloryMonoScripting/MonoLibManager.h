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

	private:
		MonoLibManager();
		virtual ~MonoLibManager();

	private:
		static std::map<std::string, AssemblyBinding> m_Assemblies;
	};
}