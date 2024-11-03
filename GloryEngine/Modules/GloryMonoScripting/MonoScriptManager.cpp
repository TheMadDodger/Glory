#include "MonoScriptManager.h"
#include "Assembly.h"

namespace Glory
{
	void MonoScriptManager::Initialize(AssemblyClass* pEngineClass, MonoObject* pEngineObject)
	{
		MonoMethod* pTypeCountMethod = pEngineClass->GetMethod(".::GetScriptTypeCount");
		MonoObject* pExcept;
		MonoObject* result = mono_runtime_invoke(pTypeCountMethod, pEngineObject, nullptr, &pExcept);
		int typeCount = *(int*)mono_object_unbox(result);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);

		m_pScriptTypes.clear();
		m_pScriptTypes.reserve((size_t)typeCount);
		m_pScriptDummies.clear();
		m_pScriptDummies.reserve((size_t)typeCount);
		m_ScriptTypeNames.clear();
		m_ScriptTypeNames.reserve((size_t)typeCount);

		MonoMethod* pTypeMethod = pEngineClass->GetMethod(".::GetScriptType");
		MonoMethod* pDummyMethod = pEngineClass->GetMethod(".::GetScriptDummy");
		MonoMethod* pTypeNameMethod = pEngineClass->GetMethod(".::GetScriptTypeName");

		for (int i = 0; i < typeCount; ++i)
		{
			void* args[1] = { &i };
			result = mono_runtime_invoke(pTypeMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			m_pScriptTypes.push_back(result);
			result = mono_runtime_invoke(pDummyMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			m_pScriptDummies.push_back(result);
			result = mono_runtime_invoke(pTypeNameMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			char* str = mono_string_to_utf8((MonoString*)result);
			m_ScriptTypeNames.push_back(std::string(str));
			mono_free(str);
		}
	}

	int MonoScriptManager::TypeIndexFromName(std::string_view name) const
	{
		for (size_t i = 0; i < m_ScriptTypeNames.size(); ++i)
		{
			const std::string_view type = m_ScriptTypeNames[i];
			if (name != type) continue;
			return i;
		}
		return -1;
	}

	MonoObject* MonoScriptManager::Dummy(size_t index) const
	{
		return m_pScriptDummies[index];
	}

	MonoObject* MonoScriptManager::Type(size_t index) const
	{
		return m_pScriptTypes[index];
	}

	std::string_view MonoScriptManager::TypeName(size_t index) const
	{
		return m_ScriptTypeNames[index];
	}
}
