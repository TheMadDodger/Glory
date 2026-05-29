#pragma once
#include <Object.h>

#include <engine_visibility.h>

#include <map>
#include <string>
#include <string_view>
#include <filesystem>

namespace Glory::Utils
{
    class BinaryStream;
}

namespace Glory
{
    class IEngine;
    class Resources;

    /** @brief Resource */
    class Resource : public Object
    {
    public:
        /** @brief Constructor */
        GLORY_ENGINE_API Resource();
        /** @overload */
        GLORY_ENGINE_API Resource(UUID uuid);
        /** @overload */
        GLORY_ENGINE_API Resource(const std::string& name);
        /** @overload */
        GLORY_ENGINE_API Resource(UUID uuid, const std::string& name);
        /** @overload */
        GLORY_ENGINE_API Resource(Resource&& other) noexcept;
        /** @brief Move assignment operator */
        GLORY_ENGINE_API Resource& operator=(Resource&& other) noexcept;

        /** @brief Destructor */
        GLORY_ENGINE_API virtual ~Resource();

        GLORY_ENGINE_API bool IsDirty(uint64_t compare) const;
        GLORY_ENGINE_API void IncrementDirtyVersion();
        GLORY_ENGINE_API uint64_t DirtyVersion() const;
        GLORY_ENGINE_API void SetDirtyVersion(uint64_t version);

    public:
        virtual void Serialize(Utils::BinaryStream& container) const {};
        virtual void Deserialize(Utils::BinaryStream& container) {};

        /** @brief Change the UUID of this resource
         * @param uuid The new UUID of this resource
         *
         * Only use this after constructing a new resource,
         * never use this on an existing resource!
         */
        GLORY_ENGINE_API void SetResourceUUID(UUID uuid);

        /** @brief Get a vector containing other resources referenced by this resource */
        virtual void References(IEngine* pEngine, std::vector<UUID>& references) const = 0;

        virtual void OnAddedToManager(Resources*) {};

    private:
        friend class Resources;
        friend class AssetDatabase;
        friend class LoaderModule;

        uint64_t m_DirtyVersion = 1;
    };
}
