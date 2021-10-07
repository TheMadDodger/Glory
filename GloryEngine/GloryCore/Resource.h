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

        const UUID& GetUUID() const;

    private:
        UUID m_ID;
    };
}
