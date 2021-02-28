#pragma once
#include "ResourceLoaderModule.h"
#include "FileData.h"

namespace Glory
{
    struct FileImportSettings : ImportSettings
    {
        int Flags;
    };

    class FileLoaderModule : public ResourceLoaderModule<FileData, FileImportSettings>
    {
    public:
        FileLoaderModule();
        virtual ~FileLoaderModule();
        
    protected:
        const std::type_info& GetModuleType() override;
        FileData* LoadResource(const std::string& path, const FileImportSettings& importSettings);

        virtual bool ReadFile(const std::string& path, std::vector<char>& buffer, const FileImportSettings& importSettings);

    protected:
        virtual void Initialize() override {};
        virtual void Cleanup() override {};
        virtual void Update() override {};
        virtual void Draw() override {};
    };
}
