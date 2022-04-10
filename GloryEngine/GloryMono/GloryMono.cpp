#include "GloryMono.h"

namespace Glory
{
	MonoDomain*		GloryMono::m_pMonoDomain;
	MonoAssembly*	GloryMono::m_pMonoAssembly;
	MonoImage*		GloryMono::m_pMonoImage;
	MonoObject*		GloryMono::m_pMonoObject;
	uint32_t		GloryMono::m_MonoObjectGCHandle;

	void GloryMono::Initialize()
	{
		mono_set_dirs(".", "");
		m_pMonoDomain = mono_jit_init_version("GloryMain", "v4.0.30319");
		
		m_pMonoAssembly = mono_domain_assembly_open(m_pMonoDomain, "./GloryCS.dll");

		m_pMonoImage = mono_assembly_get_image(m_pMonoAssembly);

		mono_add_internal_call("Glory.RandomWord::GetWord()", &RandomWord::GetWord);

		MonoClass* pMainClass = mono_class_from_name(m_pMonoImage, "Glory", "GloryCSMain");
		MonoMethodDesc* pMainFuncDesc = mono_method_desc_new(".GloryCSMain:main()", false);
		MonoMethod* pMainFunc = mono_method_desc_search_in_class(pMainFuncDesc, pMainClass);
		// Exception object
		MonoObject* pExObject = nullptr;
		m_pMonoObject = mono_runtime_invoke(pMainFunc, nullptr, nullptr, &pExObject);
		m_MonoObjectGCHandle = mono_gchandle_new(m_pMonoObject, false);
		mono_method_desc_free(pMainFuncDesc);
	}

	void GloryMono::Cleanup()
	{
		mono_gchandle_free(m_MonoObjectGCHandle);

		if (m_pMonoDomain)
			mono_jit_cleanup(m_pMonoDomain);

		m_pMonoDomain = nullptr;
		m_pMonoAssembly = nullptr;
		m_pMonoImage = nullptr;
	}

	MonoString* RandomWord::GetWord()
	{
		return mono_string_new(mono_domain_get(), "Hello World 42069");
	}
}
