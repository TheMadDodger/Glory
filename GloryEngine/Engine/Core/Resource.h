#pragma once
#include "Object.h"

#include <map>
#include <string>
#include <string_view>
#include <filesystem>

namespace Glory
{
    class BinaryStream;
    class Engine;

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

        /** @brief Whether the resource is dirty */
        bool IsDirty() const;
        /** @brief Set resource dirty */
        void SetDirty(bool dirty);

    public:
        virtual void Serialize(BinaryStream& container) const {};
        virtual void Deserialize(BinaryStream& container) {};

        /** @brief Change the UUID of this resource
         * @param uuid The new UUID of this resource
         *
         * Only use this after constructing a new resource,
         * never use this on an existing resource!
         */
        void SetResourceUUID(UUID uuid);

        /** @brief Get a vector containing other resources referenced by this resource */
        virtual void References(Engine* pEngine, std::vector<UUID>& references) const { }

    private:
        friend class AssetManager;
        friend class AssetDatabase;
        friend class LoaderModule;

        bool m_IsDirty;
    };
}
