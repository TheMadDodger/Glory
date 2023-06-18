#pragma once
#include <Glory.h>
#include <string>
#include <map>
#include <functional>

#include <mono/utils/mono-forward.h>
#include <mono/metadata/object-forward.h>

namespace Glory
{
	class AssemblyBinding;
	class ScriptingLib;

	class AssemblyDomain
	{
	public:
		GLORY_API MonoDomain* GetNative() const { return m_pMonoDomain; }
		GLORY_API const std::string& Name() const { return m_Name; }

		GLORY_API void LoadLib(const ScriptingLib& lib);
		GLORY_API void ReloadAll();
		GLORY_API void Cleanup();

		GLORY_API AssemblyBinding* GetAssembly(const std::string& name);
		GLORY_API AssemblyBinding* GetMainAssembly();
		GLORY_API const std::string& GetMainAssemblyName();

		GLORY_API MonoObject* InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, MonoObject** pExceptionObject, void** args);

		GLORY_API size_t AssemblyCount();
		GLORY_API void ForEachAssembly(std::function<void(AssemblyBinding*)> callback);


	private:
		AssemblyDomain(const std::string& name, MonoDomain* pMonoDomain);
		~AssemblyDomain();

	private:
		friend class MonoManager;
		const std::string m_Name;
		MonoDomain* m_pMonoDomain;

		std::map<std::string, AssemblyBinding> m_Assemblies;
		std::string m_MainAssemblyName;
	};
}
