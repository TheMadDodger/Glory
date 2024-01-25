#pragma once
#include "Resource.h"

#include <vector>
#include <algorithm>

namespace Glory
{
	class Resources;

	class BaseResourceManager
	{
	public:
		BaseResourceManager(Resources* pResources);

		virtual uint32_t Type() const = 0;
		virtual Resource* Add(Resource* pResource) = 0;
		virtual Resource* GetBase(UUID uuid) = 0;

		size_t Index(UUID uuid) const;
		bool IsLoaded(UUID uuid) const;

		Resources* Owner();

	protected:
		void SetManager(Resource* pResource);

	protected:
		Resources* m_pOwner;
		std::vector<UUID> m_UUIDs;
	};

	template<class T>
	class ResourceManager : public BaseResourceManager
	{
	public:
		ResourceManager(Resources* pResources): BaseResourceManager(pResources)
		{}

		virtual uint32_t Type() const override
		{
			return ResourceTypes::GetHash<T>();
		}

		virtual Resource* Add(Resource* pResource) override
		{
			T* pTResource = static_cast<T*>(pResource);
			T* pMoved = Add(std::move(*pTResource));
			delete pResource;
			return pMoved;
		}

		virtual Resource* GetBase(UUID uuid) override
		{
			return Get(uuid);
		}

		T* Add(T&& resource)
		{
			size_t index = m_Resources.size();
			m_Resources.push_back(std::move(resource));
			SetManager(&m_Resources[index]);
			m_UUIDs.push_back(m_Resources[index].GetUUID());
			return &m_Resources.at(index);
		}

		T* Get(UUID uuid)
		{
			const size_t index = Index(uuid);
			return index == UINT64_MAX ? nullptr : &m_Resources[index];
		}

	private:
		std::vector<T> m_Resources;
	};
}
