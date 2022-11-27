#pragma once
#include "Object.h"

namespace Glory
{
    class Resource : public Object
    {
    public:
        Resource();
        Resource(UUID uuid);
        Resource(const std::string& name);
        Resource(UUID uuid, const std::string& name);
        virtual ~Resource();

    private:
        friend class AssetManager;
        friend class AssetDatabase;
        friend class LoaderModule;
    };
}
