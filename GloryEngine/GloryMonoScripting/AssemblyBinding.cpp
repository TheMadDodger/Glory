#include "AssemblyBinding.h"
#include <mono/metadata/assembly.h>
#include <Debug.h>
#include <mono/metadata/debug-helpers.h>

namespace Glory
{
	AssemblyBinding::AssemblyBinding()
		: m_pDomain(nullptr), m_Name(""), m_pAssembly(nullptr), m_pImage(nullptr)
	{
	}

	AssemblyBinding::AssemblyBinding(MonoDomain* pDomain, const std::string& name)
		: m_pDomain(pDomain), m_Name(name), m_pAssembly(nullptr), m_pImage(nullptr)
	{
	}

	AssemblyBinding::~AssemblyBinding()
	{
	}

	void AssemblyBinding::Initialize()
	{
		m_pAssembly = mono_domain_assembly_open(m_pDomain, m_Name.c_str());
		if (m_pAssembly == nullptr) return;
		m_pImage = mono_assembly_get_image(m_pAssembly);
	}

	void AssemblyBinding::Destroy()
	{
		if (m_pImage) mono_image_close(m_pImage);
		if (m_pAssembly) mono_assembly_close(m_pAssembly);
		m_pAssembly = nullptr;
		m_pImage = nullptr;
	}

	MonoImage* AssemblyBinding::GetMonoImage()
	{
		return m_pImage;
	}

	AssemblyClass* AssemblyBinding::GetClass(const std::string& namespaceName, const std::string& className)
	{
		if (m_Namespaces.find(namespaceName) == m_Namespaces.end() || m_Namespaces[namespaceName].m_Classes.find(className) == m_Namespaces[namespaceName].m_Classes.end())
		{
			return LoadClass(namespaceName, className);
		}
		return &m_Namespaces[namespaceName].m_Classes[className];
	}

	bool AssemblyBinding::GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c)
	{
		if (m_Namespaces.find(namespaceName) == m_Namespaces.end() || m_Namespaces[namespaceName].m_Classes.find(className) == m_Namespaces[namespaceName].m_Classes.end())
		{
			if (LoadClass(namespaceName, className) == nullptr) return false;
		}
		c = m_Namespaces[namespaceName].m_Classes[className];
		return true;
	}

	const std::string& AssemblyBinding::Name()
	{
		return m_Name;
	}

	AssemblyClass* AssemblyBinding::LoadClass(const std::string& namespaceName, const std::string& className)
	{
		MonoClass* pClass = mono_class_from_name(m_pImage, namespaceName.c_str(), className.c_str());
		if (pClass == nullptr)
		{
			Debug::LogError("Failed to load mono class");
			return nullptr;
		}

		if (mono_class_init(pClass) == false)
		{
			Debug::LogError("AssemblyBinding::LoadClass > Failed to initialize a MonoClass!");
			return nullptr;
		}

		m_Namespaces[namespaceName].m_Classes[className] = AssemblyClass(className, pClass);
		return &m_Namespaces[namespaceName].m_Classes[className];
	}

	AssemblyClass::AssemblyClass() : m_Name(""), m_pClass(nullptr) {}

	AssemblyClass::AssemblyClass(const std::string& name, MonoClass* pClass) : m_Name(name), m_pClass(pClass) {}

	MonoMethod* AssemblyClass::GetMethod(const std::string& name)
	{
		if (m_pMethods.find(name) == m_pMethods.end())
		{
			return LoadMethod(name);
		}
		return m_pMethods[name];
	}

	MonoMethod* AssemblyClass::LoadMethod(const std::string& name)
	{
		MonoMethodDesc* pMainFuncDesc = mono_method_desc_new(name.c_str(), false);
		MonoMethod* pMethod = mono_method_desc_search_in_class(pMainFuncDesc, m_pClass);
		mono_method_desc_free(pMainFuncDesc);
		return pMethod;
	}
}
