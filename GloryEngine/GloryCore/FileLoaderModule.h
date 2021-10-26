#pragma once
#include "ResourceLoaderModule.h"
#include "FileData.h"

namespace Glory
{
    struct FileImportSettings : ImportSettings
    {
        FileImportSettings();
        FileImportSettings(const std::string& extension);

        int Flags;
        bool AddNullTerminateAtEnd;
    };

    class FileLoaderModule : public ResourceLoaderModule<FileData, FileImportSettings>
    {
    public:
        FileLoaderModule();
        virtual ~FileLoaderModule();

    protected:
        const std::type_info& GetModuleType() override;
        FileData* LoadResource(const std::string& path, const FileImportSettings& importSettings);
        FileData* LoadResource(const void* buffer, size_t length, const FileImportSettings& importSettings);

        virtual bool ReadFile(const std::string& path, std::vector<char>& buffer, const FileImportSettings& importSettings);

        virtual FileImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
        virtual void WriteImportSettings_Internal(const FileImportSettings& importSettings, YAML::Emitter& out) override;

    protected:
        virtual void Initialize() override {};
        virtual void Cleanup() override {};
    };
}
