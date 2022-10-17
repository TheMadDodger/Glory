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

	MonoScript::MonoScript(FileData* pFileData) : Script(pFileData)
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	void MonoScript::Invoke(Object* pObject, const std::string& method, void** args)
	{
		AssemblyClass* pClass = LoadClass(MonoLibManager::GetMainAssemblyName(), m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject);
		if (pMonoObject == nullptr) return;
		std::string fullMethodName = ".::" + method;
		MonoMethod* pMethod = pClass->GetMethod(fullMethodName);
		if (pMethod == nullptr) return;
		MonoObject* pException = nullptr;
		MonoLibManager::InvokeMethod(pMethod, pMonoObject, &pException, args);
	}

	bool MonoScript::IsBehaviour()
	{
		return false;
	}

	AssemblyClass* MonoScript::LoadClass(const std::string& lib, const std::string& namespaceName, const std::string& className)
	{
		std::string name = Name();
		AssemblyBinding* pAssembly = MonoLibManager::GetAssembly(lib);
		if (pAssembly == nullptr) return nullptr;
		AssemblyClass* pClass = pAssembly->GetClass(namespaceName, className);
		if (pClass == nullptr) return nullptr;
		m_pMonoClass = pClass->m_pClass;
		return pClass;
	}

	MonoObject* MonoScript::LoadObject(Object* pObject)
	{
		if (m_pMonoClass == nullptr) return nullptr;
		return MonoObjectManager::GetObject(m_pMonoClass, pObject);
	}
}
