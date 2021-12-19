#pragma once
#include "Object.h"

namespace Glory
{
    class Resource : public Object
    {
    public:
        Resource();
        Resource(UUID uuid);
        virtual ~Resource();

        const std::string& Name();

    private:
        std::string m_Name;
    };
}
