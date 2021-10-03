#pragma once
#include "Object.h"
#include "UUID.h"

namespace Glory
{
    class Resource : public Object
    {
    public:
        Resource();
        virtual ~Resource();

    private:
        UUID m_ID;
    };
}
