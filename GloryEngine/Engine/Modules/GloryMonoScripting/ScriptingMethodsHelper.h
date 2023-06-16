#pragma once
#include <vector>
#include <mono/jit/jit.h>
#include <Glory.h>

namespace Glory
{
	class ScriptingMethodsHelper
	{
	public:
		GLORY_API void InvokeScriptingMethod(MonoObject* pObject, const std::string& methodName, std::vector<void*>& args);

		void Initialize(MonoImage* pImage);

	private:
		ScriptingMethodsHelper() = default;
		virtual ~ScriptingMethodsHelper() = default;

		void Cleanup();

	private:
		friend class MonoManager;
		MonoClass* m_pMethodHelperClass;
		MonoMethod* m_pCallBasicMethod;
		MonoMethod* m_pCallOneArgsMethod;
		MonoMethod* m_pCallTwoArgsMethod;
	};
}
