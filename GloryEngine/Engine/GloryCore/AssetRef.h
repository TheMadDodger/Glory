#pragma once
#include "Resource.h"
#include <memory>

namespace Glory
{
	// Similar to a smart pointer, but the pointer can be changed! (for example when the asset is reloaded)
	class AssetRef
	{
	public:
		template<class T>
		T* Get()
		{
			return (T*)m_pResource;
		}

	private:
		// Should under no circumstances be copyable!
		AssetRef(const AssetRef&& other) = delete;
		const AssetRef& operator=(const AssetRef&& other) = delete;

	private:
		friend class AssetManager;
		Resource* m_pResource;
	};
}
