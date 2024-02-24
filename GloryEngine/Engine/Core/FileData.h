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
        virtual ~FileData();

        size_t Size();
        const char* Data();

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        friend class FileLoaderModule;
        std::vector<char> m_Data;
    };
}
