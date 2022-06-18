#pragma once
#include "Resource.h"
#include "FileData.h"

namespace Glory
{
    class Script : public FileData
    {
    public:
        Script();
        Script(FileData* pFileData);
        virtual ~Script();

        virtual void Invoke(Object* pObject, const std::string& method, void** args) = 0;

        virtual bool IsBehaviour() = 0;

        //virtual size_t Size() = 0;
        //virtual const char* Data() = 0;
    };
}
