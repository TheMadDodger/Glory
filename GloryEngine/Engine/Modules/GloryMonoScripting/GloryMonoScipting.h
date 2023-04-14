#pragma once
#include "MonoScript.h"
#include "AssemblyBinding.h"
#include "MonoScriptLoader.h"
#include "GloryMono.h"

#define BIND(x, y) internalCalls.push_back(InternalCall("csharp", x, &y))

namespace Glory
{
    class CoreLibManager;

	class GloryMonoScipting : public ScriptingModuleTemplate<MonoScript, MonoScriptLoader>, IScriptExtender
	{
    public:
        GloryMonoScipting();
        virtual ~GloryMonoScipting();

        void SetMonoDirs(const std::string& assemblyDir, const std::string& configDir);

        GLORY_MODULE_VERSION_H(0,1,0);

        CoreLibManager* GetCoreLibManager() const;

    private:
        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void InitializeScripting() override;
        virtual void Cleanup() override;

        void LoadLib(const ScriptingLib& library) override;
        void Bind(const InternalCall& internalCall) override;

        virtual std::string Language() override;
        virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
        virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

    private:
        CoreLibManager* m_pCoreLibManager;
	};
}
