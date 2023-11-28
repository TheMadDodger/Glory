#pragma once
#include "MonoScript.h"
#include "MonoScriptLoader.h"
#include "GloryMono.h"

#define BIND(x, y) internalCalls.push_back(InternalCall("csharp", x, &y))

REFLECTABLE_ENUM_NS(Glory, MonoLogLevel, error, critical, warning, message, info, debug);

namespace Glory
{
    class MonoManager;

	class GloryMonoScipting : public ScriptingModuleTemplate<MonoScript, MonoScriptLoader>, IScriptExtender
	{
    public:
        GloryMonoScipting();
        virtual ~GloryMonoScipting();

        GLORY_MODULE_VERSION_H(0,3,0);

        GLORY_API MonoManager* GetMonoManager() const;

    private:
        virtual void LoadSettings(ModuleSettings& settings) override;
        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void Cleanup() override;

        void AddLib(const ScriptingLib& library) override;
        void Bind(const InternalCall& internalCall) override;

        virtual std::string Language() override;
        virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
        virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

    private:
        MonoManager* m_pMonoManager;
	};
}
