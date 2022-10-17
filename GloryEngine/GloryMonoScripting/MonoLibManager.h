#pragma once
#include <map>
#include <ScriptingExtender.h>
#include "AssemblyBinding.h"

namespace Glory
{
	class MonoLibManager
	{
	public:
		static GLORY_API void LoadLib(MonoDomain* pDomain, const ScriptingLib& lib);
		static GLORY_API AssemblyBinding* GetAssembly(const std::string& name);
		static GLORY_API AssemblyBinding* GetMainAssembly();
		static GLORY_API const std::string& GetMainAssemblyName();

		static GLORY_API MonoObject* InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args);

		static GLORY_API void GetAllClasses();

		static GLORY_API size_t AssemblyCount();
		static GLORY_API void ForEachAssembly(std::function<void(AssemblyBinding*)> callback);

	private:
		MonoLibManager();
		virtual ~MonoLibManager();

		static void Cleanup();

	private:
		friend class MonoManager;
		static std::map<std::string, AssemblyBinding> m_Assemblies;
		static std::string m_MainAssemblyName;
	};
}