#pragma once
#include <IScriptExtender.h>
#include <Glory.h>

namespace Glory
{
	class Engine;

	extern "C" GLORY_API void LoadExtension(Engine* pEngine);

	class EntitiesMonoExtender : public IScriptExtender
	{
	public:
		virtual std::string Language() override;
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(std::vector<ScriptingLib>& libs) override;
	};
}
