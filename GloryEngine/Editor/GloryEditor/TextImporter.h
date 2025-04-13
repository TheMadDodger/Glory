#pragma once
#include "ImporterTemplate.h"

#include <TextFileData.h>

namespace Glory::Editor
{
    class TextImporter : public ImporterTemplate<TextFileData>
    {
    public:
        TextImporter();
        virtual ~TextImporter();

    private:
        virtual std::string_view Name() const override;
        void Initialize() override;
        void Cleanup() override;

    private:
        bool SupportsExtension(const std::filesystem::path& extension) const override;
        ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
        virtual bool SaveResource(const std::filesystem::path& path, TextFileData* pResource) const override;

        bool ReadFile(const std::filesystem::path& path, std::vector<char>& buffer) const;
    };
}
