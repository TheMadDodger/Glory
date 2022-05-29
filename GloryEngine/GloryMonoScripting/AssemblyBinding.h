#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <map>

namespace Glory
{
	struct AssemblyNamespace
	{
		std::string m_Name;
		std::map<std::string, MonoClass*> m_pClasses;
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
		MonoClass* GetClass(const std::string& namespaceName, const std::string& className);

	private:
		MonoClass* LoadClass(const std::string& namespaceName, const std::string& className);

	private:
		MonoDomain* m_pDomain;
		const std::string m_Name;
		MonoAssembly* m_pAssembly;
		MonoImage* m_pImage;
		std::map<std::string, AssemblyNamespace> m_Namespaces;
	};
}
