#include "AssemblyBinding.h"
#include <mono/metadata/assembly.h>

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
}
