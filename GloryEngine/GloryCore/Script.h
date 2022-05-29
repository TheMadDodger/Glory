#pragma once
#include "Resource.h"
#include "FileData.h"

namespace Glory
{
    class Script : public FileData
    {
    public:
        Script();
        virtual ~Script();

        virtual void Invoke(Object* pObject, const std::string& method) = 0;

        //virtual size_t Size() = 0;
        //virtual const char* Data() = 0;
    };
}
