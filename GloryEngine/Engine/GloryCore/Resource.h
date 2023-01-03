#pragma once
#include "Object.h"
#include <map>
#include <string>
#include <string_view>
#include <filesystem>

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

    public:
        const size_t SubResourceCount() const;
        Resource* Subresource(size_t index) const;
        Resource* Subresource(std::string_view name) const;

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

        void AddSubresource(Resource* pResource, const std::string& name);

    protected:
        std::vector<Resource*> m_pSubresources;
        std::map<std::string, size_t, std::less<>> m_NameToSubresourceIndex;
        Resource* m_pParent;

    private:
        friend class AssetManager;
        friend class AssetDatabase;
        friend class LoaderModule;
    };
}
