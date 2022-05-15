#pragma once
#include "Engine.h"
#include "IScriptExtender.h"

namespace Glory
{
	class ScriptingExtender
	{
	public:
		template<class T, typename... Args>
		void RegisterExtender(Args&&... args)
		{
			m_pManagedExtenders.push_back(new T(args...));
		}

		void RegisterExtender(IScriptExtender* pExtender);

	private:
		friend class Engine;
		ScriptingExtender();
		virtual ~ScriptingExtender();

		void Initialize(Engine* pEngine);
		void LoadExtender(IScriptExtender* pExtender);

	private:
		std::vector<IScriptExtender*> m_pManagedExtenders;
		std::vector<IScriptExtender*> m_pExtenders;

		std::vector<InternalCall> m_InternalCalls;
		std::vector<ScriptingLib> m_Libs;
	};
}
