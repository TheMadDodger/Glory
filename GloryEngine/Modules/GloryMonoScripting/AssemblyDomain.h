#pragma once
#include <Glory.h>
#include <string>
#include <map>
#include <functional>

#include <mono/utils/mono-forward.h>
#include <mono/metadata/object-forward.h>

namespace Glory
{
	class Assembly;
	class ScriptingLib;
	class MonoScriptObjectManager;

	class AssemblyDomain
	{
	public:
		GLORY_API MonoDomain* GetMonoDomain() const { return m_pMonoDomain; }
		GLORY_API const std::string& Name() const { return m_Name; }

		GLORY_API void LoadLib(const ScriptingLib& lib);

		GLORY_API void Reload(const std::string& name);
		GLORY_API void Unload(bool isReloading = false);

		GLORY_API Assembly* GetAssembly(const std::string& name);
		GLORY_API Assembly* GetMainAssembly();
		GLORY_API const std::string& GetMainAssemblyName();
		GLORY_API MonoScriptObjectManager* ScriptObjectManager();

		GLORY_API MonoObject* InvokeMethod(MonoMethod* pMethod, MonoObject* pObject, void** args);

		GLORY_API size_t AssemblyCount();
		GLORY_API void ForEachAssembly(std::function<void(Assembly*)> callback);

		GLORY_API bool SetCurrentDomain(bool force = false);

	private:
		AssemblyDomain(const std::string& name, MonoDomain* pMonoDomain);
		~AssemblyDomain();

	private:
		friend class MonoManager;
		const std::string m_Name;
		MonoDomain* m_pMonoDomain;
		MonoScriptObjectManager* m_pScriptObjectManager;

		std::map<std::string, Assembly> m_Assemblies;
		std::string m_MainAssemblyName;
	};
}
