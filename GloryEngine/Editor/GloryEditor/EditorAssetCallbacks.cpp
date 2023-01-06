#include "EditorAssetCallbacks.h"
#include "GloryContext.h"
#include "EditorAssetDatabase.h"

namespace Glory::Editor
{
	ThreadedVector<std::vector<ASSET_CALLBACK>> EditorAssetCallbacks::m_RegisteredCallbacks;
	ThreadedVector<EditorAssetCallbacks::EnqueuedCallback> EditorAssetCallbacks::m_EnqueuedCallbacks;

	void EditorAssetCallbacks::RegisterCallback(const AssetCallbackType& type, ASSET_CALLBACK callback)
	{
		m_RegisteredCallbacks.Do((size_t)type, [&](std::vector<ASSET_CALLBACK>* callbacks) { callbacks->push_back(callback); });
	}

	void EditorAssetCallbacks::Initialize()
	{
		for (size_t i = 0; i < (size_t)AssetCallbackType::CT_MAX; i++)
		{
			m_RegisteredCallbacks.push_back(std::vector<ASSET_CALLBACK>());
		}
	}

	void EditorAssetCallbacks::Cleanup()
	{
		m_RegisteredCallbacks.Clear();
	}

	void EditorAssetCallbacks::TriggerCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
		m_RegisteredCallbacks.Do((size_t)type, [&](const std::vector<ASSET_CALLBACK>& callbacks)
		{
			for (size_t i = 0; i < callbacks.size(); i++)
			{
				callbacks[i](uuid, meta, pResource);
			}
		});
	}

	void EditorAssetCallbacks::RunCallbacks()
	{
		m_EnqueuedCallbacks.ForEachClear([&](const EnqueuedCallback& callback)
		{
			TriggerCallback(callback.m_Type, callback.m_UUID, callback.m_pResource);
		});
	}

	void EditorAssetCallbacks::EnqueueCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource)
	{
		EnqueuedCallback callback = EnqueuedCallback();
		callback.m_Type = type;
		callback.m_UUID = uuid;
		callback.m_pResource = pResource;
		m_EnqueuedCallbacks.push_back(callback);
	}

	EditorAssetCallbacks::EditorAssetCallbacks() {}
	EditorAssetCallbacks::~EditorAssetCallbacks() {}
}
