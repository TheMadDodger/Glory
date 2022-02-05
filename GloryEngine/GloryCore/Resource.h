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
        void SetName(const std::string& name);

    private:
        friend class AssetManager;
        std::string m_Name;
    };
}
