#pragma once
#include "Engine.h"

namespace Glory
{
	class IScriptExtender;
	class ScriptingLib;
	class InternalCall;
	class GloryMonoScipting;

	class ScriptingExtender
	{
	public:
		GLORY_API void RegisterExtender(IScriptExtender* pExtender);
		GLORY_API void AddInternalLib(const std::string& location, const std::string& name, void* data);
		GLORY_API size_t InternalLibCount() const;
		GLORY_API const ScriptingLib& GetInternalLib(size_t index) const;

	private:
		friend class GloryMonoScipting;
		ScriptingExtender();
		virtual ~ScriptingExtender();

		void Initialize(GloryMonoScipting* pScripting);

	private:
		std::vector<IScriptExtender*> m_pExtenders;
		std::vector<InternalCall> m_InternalCalls;
		std::vector<ScriptingLib> m_Libs;
	};
}
