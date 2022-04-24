#pragma once
#include <mono/jit/jit.h>
#include <string>

namespace Glory
{
	class AssemblyBinding
	{
	public:
		AssemblyBinding();
		AssemblyBinding(MonoDomain* pDomain, const std::string& name);
		virtual ~AssemblyBinding();

		void Initialize();
		void Destroy();

		MonoImage* GetMonoImage();

	private:
		MonoDomain* m_pDomain;
		const std::string m_Name;
		MonoAssembly* m_pAssembly;
		MonoImage* m_pImage;
	};
}
