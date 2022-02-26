#pragma once
#include "ResourceLoaderModule.h"
#include "FileData.h"
#include "FileLoaderModule.h"
#include "ShaderSourceData.h"
#include <functional>

namespace Glory
{
    struct ShaderSourceImportSettings : public ImportSettings
    {
        ShaderSourceImportSettings();
        ShaderSourceImportSettings(const std::string& extension);
    };

    class ShaderSourceLoaderModule : public ResourceLoaderModule<ShaderSourceData, ShaderSourceImportSettings>
    {
    public:
        ShaderSourceLoaderModule();
        virtual ~ShaderSourceLoaderModule();

        static bool GetShaderTypeFromString(const std::string& typeString, ShaderType& shaderType);

    protected:
        const std::type_info& GetModuleType() override;
        ShaderSourceData* LoadResource(const std::string& path, const ShaderSourceImportSettings& importSettings);
        ShaderSourceData* LoadResource(const void* buffer, size_t length, const ShaderSourceImportSettings& importSettings);

        virtual bool ReadFile(const std::string& path, ShaderSourceData* pShaderSource, const ShaderSourceImportSettings& importSettings);

        virtual ShaderSourceImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
        virtual void WriteImportSettings_Internal(const ShaderSourceImportSettings& importSettings, YAML::Emitter& out) override;

    protected:
        virtual void Initialize() override {};
        virtual void Cleanup() override {};

    private:
        void ProcessLine(const std::string& path, ShaderSourceData* pShaderSource, const std::string& line);
        bool ProcessSymbol(const std::string& path, ShaderSourceData* pShaderSource, const std::string& symbol, const std::string& argument);
        void AppendLine(const std::string& line, std::vector<char>& buffer);

    private: // Symbol callbacks

    private:
        std::map<std::string, std::function<bool(const std::string&, ShaderSourceData*, const std::string&)>> m_SymbolCallbacks;
        static std::map<std::string, ShaderType> m_ShaderTypes;
    };
}
