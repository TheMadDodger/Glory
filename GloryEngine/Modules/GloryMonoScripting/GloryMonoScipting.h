#pragma once
#include "MonoScript.h"
#include "MonoScriptLoader.h"
#include "GloryMono.h"

#define BIND(x, y) internalCalls.push_back(InternalCall(x, &y))

REFLECTABLE_ENUM_NS(Glory, MonoLogLevel, error, critical, warning, message, info, debug);

namespace Glory
{
    class MonoManager;

	class GloryMonoScipting : public Module
	{
    public:
        GloryMonoScipting();
        virtual ~GloryMonoScipting();

        GLORY_MODULE_VERSION_H(0,4,0);

        GLORY_API MonoManager* GetMonoManager() const;

        GLORY_API const std::type_info& GetModuleType() override;

        GLORY_API ScriptingExtender* GetScriptingExtender();

    private:
        virtual void LoadSettings(ModuleSettings& settings) override;
        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void Cleanup() override;

        void GetInternalCalls(std::vector<InternalCall>& internalCalls);
        void GetLibs(ScriptingExtender* pScriptingExtender);

        void AddLib(const ScriptingLib& library);
        void Bind(const InternalCall& internalCall);

    private:
        friend class ScriptingExtender;
        MonoManager* m_pMonoManager;
        ScriptingExtender* m_pScriptingExtender;
	};
}
