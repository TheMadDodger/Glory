#pragma once
#include <vector>
#include <string>
#include <mono/metadata/object-forward.h>

namespace Glory
{
	struct AssemblyClass;

	class MonoScriptManager
	{
	public:
		void Initialize(AssemblyClass* pEngineClass, MonoObject* pEngineObject);
		int TypeIndexFromName(std::string_view name) const;
		MonoObject* Dummy(size_t index) const;
		MonoObject* Type(size_t index) const;
		std::string_view TypeName(size_t index) const;

	public:
		MonoScriptManager() {}
		virtual ~MonoScriptManager() = default;

	private:
		std::vector<MonoObject*> m_pScriptTypes;
		std::vector<MonoObject*> m_pScriptDummies;
		std::vector<std::string> m_ScriptTypeNames;
	};
}
