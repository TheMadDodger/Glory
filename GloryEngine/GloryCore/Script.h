#pragma once
#include "Resource.h"

namespace Glory
{
    class Script : public Resource
    {
    public:
        Script();
        virtual ~Script();

        //virtual size_t Size() = 0;
        //virtual const char* Data() = 0;
    };
}
