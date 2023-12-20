#pragma once
#include <ResourceLoaderModule.h>
#include <yaml-cpp/yaml.h>

#include "MonoScript.h"

namespace Glory
{
    struct ScriptImportSettings : public ImportSettings
    {
        ScriptImportSettings() {}
        ScriptImportSettings(const std::string& extension) : ImportSettings(extension) {}
    };

	class MonoScriptLoader : public ResourceLoaderModule<MonoScript, ScriptImportSettings>
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


        std::string Find(const std::string& source, const std::string& toFind);
	};
}
