#pragma once
#include "Object.h"

#include <map>
#include <string>
#include <string_view>
#include <filesystem>

namespace Glory
{
    class BinaryStream;

    /** @brief Resource */
    class Resource : public Object
    {
    public:
        /** @brief Constructor */
        Resource();
        /** @overload */
        Resource(UUID uuid);
        /** @overload */
        Resource(const std::string& name);
        /** @overload */
        Resource(UUID uuid, const std::string& name);
        /** @brief Destructor */
        virtual ~Resource();

    public:
        virtual void Serialize(BinaryStream& container) const {};
        virtual void Deserialize(BinaryStream& container) const {};

        /** @brief Change the UUID of this resource
         * @param uuid The new UUID of this resource
         *
         * Only use this after constructing a new resource,
         * never use this on an existing resource!
         */
        void SetResourceUUID(UUID uuid);

    private:
        friend class AssetManager;
        friend class AssetDatabase;
        friend class LoaderModule;
    };
}
