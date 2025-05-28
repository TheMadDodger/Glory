#pragma once
#include "FileData.h"

namespace Glory
{
    class TextFileData : public FileData
    {
    public:
        TextFileData();
        TextFileData(FileData* pFileData);
        TextFileData(std::vector<char>&& data);
        TextFileData(std::string_view data);
        virtual ~TextFileData() = default;
    };
}