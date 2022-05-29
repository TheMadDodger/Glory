#include "MonoScript.h"
#include <AssetDatabase.h>
#include "MonoObjectManager.h"
#include "MonoLibManager.h"

namespace Glory
{
	MonoScript::MonoScript() : m_pMonoClass(nullptr)
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	void MonoScript::Invoke(Object* pObject, const std::string& method)
	{
		if (m_pMonoClass == nullptr) LoadClass("Assembly-CSharp", "");
		MonoObject* pMonoObject = MonoObjectManager::GetObject(m_pMonoClass, pObject);
		mono_class_get_method_from_name(m_pMonoClass, method.c_str(), 0);
	}

	void MonoScript::LoadClass(const std::string& lib, const std::string& namespaceName)
	{
		std::string name = Name();
		AssemblyBinding* pAssembly = MonoLibManager::GetAssembly(lib);
		m_pMonoClass = pAssembly->GetClass(namespaceName, name);
	}
}
