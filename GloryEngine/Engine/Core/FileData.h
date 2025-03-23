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

        void SetMetaData(const void* data, size_t size);

        template<typename T>
        void SetMetaData(const T& value)
        {
            SetMetaData(&value, sizeof(T));
        }

        template<typename T>
        const T& GetMetaData() const
        {
            return *reinterpret_cast<const T*>(m_MetaData.data());
        }

    private:
        virtual void References(Engine*, std::vector<UUID>&) const override {}

    private:
        friend class FileLoaderModule;
        std::vector<char> m_Data;
        std::vector<char> m_MetaData;
    };
}
