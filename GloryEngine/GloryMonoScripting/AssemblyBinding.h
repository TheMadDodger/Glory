#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <map>
#include <array>

namespace Glory
{
	struct AssemblyClass
	{
	public:
		AssemblyClass();
		AssemblyClass(const std::string& name, MonoClass* pClass);

		std::string m_Name;
		MonoClass* m_pClass;
		std::map<std::string, MonoMethod*> m_pMethods;

		MonoMethod* GetMethod(const std::string& name);

	private:
		MonoMethod* LoadMethod(const std::string& name);
	};

	struct AssemblyNamespace
	{
		std::string m_Name;
		std::map<std::string, AssemblyClass> m_Classes;
	};

	class AssemblyBinding
	{
	public:
		AssemblyBinding();
		AssemblyBinding(MonoDomain* pDomain, const std::string& name);
		virtual ~AssemblyBinding();

		void Initialize();
		void Destroy();

		MonoImage* GetMonoImage();
		AssemblyClass* GetClass(const std::string& namespaceName, const std::string& className);
		bool GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c);

	private:
		AssemblyClass* LoadClass(const std::string& namespaceName, const std::string& className);

	private:
		MonoDomain* m_pDomain;
		const std::string m_Name;
		MonoAssembly* m_pAssembly;
		MonoImage* m_pImage;
		std::map<std::string, AssemblyNamespace> m_Namespaces;
	};
}
