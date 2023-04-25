#pragma once
#include <vector>
#include <mono/jit/jit.h>
#include <Glory.h>

namespace Glory
{
	class ScriptingMethodsHelper
	{
	public:
		GLORY_API static void InvokeScriptingMethod(MonoObject* pObject, const std::string& methodName, std::vector<void*>& args);

	private:
		static void Initialize(MonoImage* pImage);

	private:
		ScriptingMethodsHelper() = default;
		virtual ~ScriptingMethodsHelper() = default;

	private:
		friend class CoreLibManager;
		static MonoClass* m_pMethodHelperClass;
		static MonoMethod* m_pCallBasicMethod;
		static MonoMethod* m_pCallOneArgsMethod;
		static MonoMethod* m_pCallTwoArgsMethod;
	};
}
