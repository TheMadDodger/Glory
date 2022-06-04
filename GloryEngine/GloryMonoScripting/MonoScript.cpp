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
		AssemblyClass* pClass = LoadClass("GloryEngine.Core.dll", "GloryEngine", "Behaviour");
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject);
		std::string fullMethodName = ".::" + method;
		MonoMethod* pMethod = pClass->GetMethod(fullMethodName);
		if (pMethod == nullptr) return;
		MonoObject* pException = nullptr;
		MonoLibManager::InvokeMethod(pMethod, pMonoObject, &pException, nullptr);
	}

	AssemblyClass* MonoScript::LoadClass(const std::string& lib, const std::string& namespaceName, const std::string& className)
	{
		std::string name = Name();
		AssemblyBinding* pAssembly = MonoLibManager::GetAssembly(lib);
		if (pAssembly == nullptr) return nullptr;
		AssemblyClass* pClass = pAssembly->GetClass(namespaceName, className);
		m_pMonoClass = pClass->m_pClass;
		return pClass;
	}

	MonoObject* MonoScript::LoadObject(Object* pObject)
	{
		if (m_pMonoClass == nullptr) return nullptr;
		return MonoObjectManager::GetObject(m_pMonoClass, pObject);
	}
}
