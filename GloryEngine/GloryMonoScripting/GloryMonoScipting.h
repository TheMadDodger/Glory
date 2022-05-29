#pragma once
#include <ScriptingModule.h>
#include <mono/jit/jit.h>
#include "MonoScript.h"
#include "AssemblyBinding.h"
#include "MonoScriptLoader.h"
#include <ScriptingExtender.h>

#define BIND(x, y) internalCalls.push_back(InternalCall("C#", x, &y))

namespace Glory
{
	class GloryMonoScipting : public ScriptingModuleTemplate<MonoScript, MonoScriptLoader>, IScriptExtender
	{
    public:
        GloryMonoScipting();
        virtual ~GloryMonoScipting();

        void SetMonoDirs(const std::string& assemblyDir, const std::string& configDir);

    private:
        virtual void Initialize() override;
        virtual void PostInitialize() override;
        virtual void InitializeScripting() override;
        virtual void Cleanup() override;

        void LoadLib(const ScriptingLib& library) override;
        void Bind(const InternalCall& internalCall) override;

        virtual std::string Language() override;
        virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
        virtual void GetLibs(std::vector<ScriptingLib>& libs) override;
	};
}
