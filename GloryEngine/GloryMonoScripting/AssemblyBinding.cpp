#include "AssemblyBinding.h"
#include <mono/metadata/assembly.h>
#include <Debug.h>

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
		m_pImage = mono_assembly_get_image(m_pAssembly);
	}

	void AssemblyBinding::Destroy()
	{
	}

	MonoImage* AssemblyBinding::GetMonoImage()
	{
		return m_pImage;
	}

	MonoClass* AssemblyBinding::GetClass(const std::string& namespaceName, const std::string& className)
	{
		if (m_Namespaces.find(namespaceName) == m_Namespaces.end() || m_Namespaces[namespaceName].m_pClasses.find(className) == m_Namespaces[namespaceName].m_pClasses.end())
		{
			return LoadClass(namespaceName, className);
		}

		return m_Namespaces[namespaceName].m_pClasses[className];
	}

	MonoClass* AssemblyBinding::LoadClass(const std::string& namespaceName, const std::string& className)
	{
		MonoClass* pClass = mono_class_from_name(m_pImage, namespaceName.c_str(), className.c_str());
		if (pClass == nullptr)
		{
			Debug::LogError("Failed to load mono class");
			return nullptr;
		}
		m_Namespaces[namespaceName].m_pClasses[className] = pClass;
		return pClass;
	}
}
