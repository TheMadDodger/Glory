#pragma once
#include "Engine.h"

namespace Glory
{
	class IScriptExtender;
	class ScriptingLib;
	class InternalCall;

	class ScriptingExtender
	{
	public:
		template<class T, typename... Args>
		void RegisterExtender(Module* pOwner, Args&&... args)
		{
			IScriptExtender* pExtender = new T(args...);
			m_pManagedExtenders.push_back(pExtender);
			m_pOwners[pExtender] = pOwner;
		}

		void RegisterExtender(Module* pOwner, IScriptExtender* pExtender);
		void RegisterManagedExtender(Module* pOwner, IScriptExtender* pExtender);

		void AddInternalLib(const std::string& name, void* data);

		size_t InternalLibCount() const;
		const ScriptingLib& GetInternalLib(size_t index) const;

	private:
		friend class Engine;
		ScriptingExtender();
		virtual ~ScriptingExtender();

		void Initialize(Engine* pEngine);
		void LoadExtender(IScriptExtender* pExtender);

	private:
		std::vector<IScriptExtender*> m_pManagedExtenders;
		std::vector<IScriptExtender*> m_pExtenders;

		std::map<IScriptExtender*, Module*> m_pOwners;

		std::vector<InternalCall> m_InternalCalls;
		std::vector<ScriptingLib> m_Libs;

		std::string m_CurrentAppendingLanguage;
		Module* m_pCurrentAppendingModule;
	};
}
