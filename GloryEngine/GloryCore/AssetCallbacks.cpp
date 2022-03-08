#include "AssetCallbacks.h"
#include "AssetDatabase.h"

namespace Glory
{
	ThreadedVector<std::vector<ASSET_CALLBACK>> AssetCallbacks::m_RegisteredCallbacks;

	void AssetCallbacks::RegisterCallback(const CallbackType& type, ASSET_CALLBACK callback)
	{
		m_RegisteredCallbacks.Do((size_t)type, [&](std::vector<ASSET_CALLBACK>* callbacks) { callbacks->push_back(callback); });
	}

	void AssetCallbacks::Initialize()
	{
		for (size_t i = 0; i < (size_t)CallbackType::CT_MAX; i++)
		{
			m_RegisteredCallbacks.push_back(std::vector<ASSET_CALLBACK>());
		}
	}

	void AssetCallbacks::Cleanup()
	{
		m_RegisteredCallbacks.Clear();
	}

	void AssetCallbacks::TriggerCallback(const CallbackType& type, UUID uuid, Resource* pResource)
	{
		ResourceMeta meta;
		if (!AssetDatabase::GetResourceMeta(uuid, meta)) return;
		m_RegisteredCallbacks.Do((size_t)type, [&](const std::vector<ASSET_CALLBACK>& callbacks)
		{
			for (size_t i = 0; i < callbacks.size(); i++)
			{
				callbacks[i](uuid, meta, pResource);
			}
		});
	}

	void AssetCallbacks::RunCallbacks()
	{
		m_EnqueuedCallbacks.ForEachClear([&](const EnqueuedCallback& callback)
		{
			TriggerCallback(callback.m_Type, callback.m_UUID, callback.m_pResource);
		});
	}

	void AssetCallbacks::EnqueueCallback(const CallbackType& type, UUID uuid, Resource* pResource)
	{
		EnqueuedCallback callback = EnqueuedCallback();
		callback.m_Type = type;
		callback.m_UUID = uuid;
		callback.m_pResource = pResource;
		m_EnqueuedCallbacks.push_back(callback);
	}

	AssetCallbacks::AssetCallbacks() {}
	AssetCallbacks::~AssetCallbacks() {}
}
