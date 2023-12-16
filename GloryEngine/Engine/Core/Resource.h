#pragma once
#include "Object.h"

#include <map>
#include <string>
#include <string_view>
#include <filesystem>

namespace Glory
{
    class BinaryStream;
    class BaseResourceManager;

    class Resource : public Object
    {
    public:
        Resource();
        Resource(UUID uuid);
        Resource(const std::string& name);
        Resource(UUID uuid, const std::string& name);
        virtual ~Resource();

    public:
        const size_t SubResourceCount() const;
        Resource* Subresource(size_t index) const;
        Resource* Subresource(std::string_view name) const;
        Resource* ParentResource() const;

        Resource* SubresourceFromPath(std::filesystem::path path);

        template<class T>
        T* Subresource(size_t index) const
        {
            return static_cast<T*>(Subresource(index));
        }

        template<class T>
        T* Subresource(std::string_view name) const
        {
            return static_cast<T*>(Subresource(name));
        }

        void AddSubresource(Resource* pResource);
        void AddSubresource(Resource* pResource, const std::string& name);

        virtual void Serialize(BinaryStream& container) const {};
        virtual void Deserialize(BinaryStream& container) {};

        /** @brief Change the UUID of this resource
         * @param uuid The new UUID of this resource
         *
         * Only use this after constructing a new resource,
         * never use this on an existing resource!
         */
        void SetResourceUUID(UUID uuid);

    protected:
        std::vector<Resource*> m_pSubresources;
        std::map<std::string, size_t, std::less<>> m_NameToSubresourceIndex;
        Resource* m_pParent;
        BaseResourceManager* m_pManager;

    private:
        friend class AssetDatabase;
        friend class LoaderModule;
        friend class BaseResourceManager;
    };
}
