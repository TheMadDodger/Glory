#include "ScriptingMethodsHelper.h"
#include "MonoManager.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Glory
{
	void ScriptingMethodsHelper::InvokeScriptingMethod(MonoObject* pObject, const std::string& methodName, std::vector<void*>& args)
	{
		/* Unfortunately this is slow and proned to crashing, so will not be used anymore */
		throw new std::exception("ScriptingMethodsHelper::InvokeScriptingMethod should not be called anymore");

		const size_t argCount = args.size();

		MonoString* pMonoString = mono_string_new(mono_domain_get(), methodName.c_str());
		args.insert(args.begin(), pMonoString);
		args.insert(args.begin(), pObject);
		MonoObject* pExcept;

		switch (argCount)
		{
		case 0:
			mono_runtime_invoke(m_pCallBasicMethod, nullptr, args.data(), &pExcept);
		break;
		case 1:
			mono_runtime_invoke(m_pCallOneArgsMethod, nullptr, args.data(), &pExcept);
			break;
		case 2:
			mono_runtime_invoke(m_pCallTwoArgsMethod, nullptr, args.data(), &pExcept);
			break;
		default:
			mono_runtime_invoke(m_pCallBasicMethod, nullptr, args.data(), &pExcept);
			break;
		}
	}

	void ScriptingMethodsHelper::Initialize(MonoImage* pImage)
	{
		m_pMethodHelperClass = mono_class_from_name(pImage, "GloryEngine", "ScriptingMethodsHelper");
		mono_class_init(m_pMethodHelperClass);

		MonoMethodDesc* pMethodDesc = mono_method_desc_new(".::CallBasicMethod(object,string)", false);
		m_pCallBasicMethod = mono_method_desc_search_in_class(pMethodDesc, m_pMethodHelperClass);
		mono_method_desc_free(pMethodDesc);

		pMethodDesc = mono_method_desc_new(".::Call1ArgsMethod", false);
		m_pCallOneArgsMethod = mono_method_desc_search_in_class(pMethodDesc, m_pMethodHelperClass);
		mono_method_desc_free(pMethodDesc);

		pMethodDesc = mono_method_desc_new(".::Call2ArgsMethod", false);
		m_pCallTwoArgsMethod = mono_method_desc_search_in_class(pMethodDesc, m_pMethodHelperClass);
		mono_method_desc_free(pMethodDesc);
	}

	void ScriptingMethodsHelper::Cleanup()
	{
		m_pMethodHelperClass = nullptr;
		m_pCallBasicMethod = nullptr;
		m_pCallOneArgsMethod = nullptr;
		m_pCallTwoArgsMethod = nullptr;
	}
}
