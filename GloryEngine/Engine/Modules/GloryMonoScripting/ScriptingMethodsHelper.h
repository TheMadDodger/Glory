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
		ScriptingMethodsHelper()
			: m_pMethodHelperClass(nullptr), m_pCallBasicMethod(nullptr), m_pCallOneArgsMethod(nullptr), m_pCallTwoArgsMethod(nullptr) {}
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
