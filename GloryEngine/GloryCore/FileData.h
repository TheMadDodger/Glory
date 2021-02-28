#pragma once
#include "Resource.h"
#include <vector>

namespace Glory
{
    class FileData : public Resource
    {
    public:
        FileData();
        virtual ~FileData();

        size_t Size();
        const char* Data();

    private:
        friend class FileLoaderModule;
        std::vector<char> m_Data;
    };
}
