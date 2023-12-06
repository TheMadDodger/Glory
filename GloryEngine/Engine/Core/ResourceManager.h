#pragma once
#include "Resource.h"

#include <vector>
#include <algorithm>

namespace Glory
{
	class BaseResourceManager
	{
	public:
		virtual uint32_t Type() const = 0;
		virtual void Add(Resource* pResource) = 0;

	protected:
		std::vector<UUID> m_UUIDs;
	};

	template<class T>
	class ResourceManager : public BaseResourceManager
	{
	public:
		virtual uint32_t Type() const override
		{
			return ResourceType::GetHash<T>();
		}

		virtual void Add(Resource* pResource) override
		{
			T* pTResource = static_cast<T*>(pResource);
			Add(std::move(*pTResource));
			delete pResource;
		}

		void Add(T&& resource)
		{
			size_t index = m_Resources.size();
			m_Resources.push_back(std::move(resource));
			m_UUIDs.push_back(m_Resources[index].GetUUID());
		}

		T* Get(UUID uuid)
		{
			const auto itor = std::find(m_UUIDs.begin(), m_UUIDs.end(), uuid);
			if (itor == m_UUIDs.end())
			{
				return nullptr;
			}
		}

	private:
		std::vector<T> m_Resources;
	};
}
