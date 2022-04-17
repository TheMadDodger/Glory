#pragma once
#include <ScriptLoaderModule.h>
#include "MonoScript.h"
#include <yaml-cpp/yaml.h>

namespace Glory
{
	class MonoScriptLoader : public ScriptLoaderModule<MonoScript, ScriptImportSettings>
	{
    public:
        MonoScriptLoader();
        virtual ~MonoScriptLoader();

    protected:
        virtual ScriptImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
        virtual void WriteImportSettings_Internal(const ScriptImportSettings& importSettings, YAML::Emitter& out) override;

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;

    private:
        virtual MonoScript* LoadResource(const std::string& path, const ScriptImportSettings& importSettings) override;
        virtual MonoScript* LoadResource(const void* buffer, size_t length, const ScriptImportSettings& importSettings) override;
        virtual void SaveResource(const std::string& path, MonoScript* pResource) override;
	};
}
