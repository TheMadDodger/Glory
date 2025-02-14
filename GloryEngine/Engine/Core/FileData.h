#pragma once
#include "Resource.h"
#include <vector>

namespace Glory
{
    class FileData : public Resource
    {
    public:
        FileData();
        FileData(FileData* pFileData);
        FileData(std::vector<char>&& data);
        FileData(std::string_view data);
        virtual ~FileData();

        size_t Size();
        const char* Data();

        std::vector<char>::iterator Begin();
        const std::vector<char>::const_iterator Begin() const;
        std::vector<char>::iterator End();
        const std::vector<char>::const_iterator End() const;

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        friend class FileLoaderModule;
        std::vector<char> m_Data;
    };
}
