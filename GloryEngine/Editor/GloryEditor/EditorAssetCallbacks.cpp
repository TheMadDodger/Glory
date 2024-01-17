#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Dispatcher.h"

namespace Glory::Editor
{
	ThreadedVar<Dispatcher<AssetCallbackData>> AssetCallbacks[(size_t)AssetCallbackType::CT_MAX];

	ThreadedVector<AssetCallbackData> EditorAssetCallbacks::m_EnqueuedCallbacks;

	UUID EditorAssetCallbacks::RegisterCallback(const AssetCallbackType& type, std::function<void(const AssetCallbackData&)> callback)
	{
		UUID uuid;
		AssetCallbacks[(size_t)type].Do([&](Dispatcher<AssetCallbackData>* dispatcher) { uuid = dispatcher->AddListener(callback); });
		return uuid;
	}

	void EditorAssetCallbacks::RemoveCallback(const AssetCallbackType& type, UUID& id)
	{
		AssetCallbacks[(size_t)type].Do([&](Dispatcher<AssetCallbackData>* dispatcher) { dispatcher->RemoveListener(id); });
	}

	void EditorAssetCallbacks::TriggerCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
		AssetCallbacks[(size_t)type].Do([&](Dispatcher<AssetCallbackData>* dispatcher)
		{
			dispatcher->Dispatch(AssetCallbackData{ type, uuid, pResource });
		});
	}

	void EditorAssetCallbacks::RunCallbacks()
	{
		m_EnqueuedCallbacks.ForEachClear([&](const AssetCallbackData& callback)
		{
			TriggerCallback(callback.m_Type, callback.m_UUID, callback.m_pResource);
		});
	}

	void EditorAssetCallbacks::EnqueueCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource)
	{
		AssetCallbackData callback = AssetCallbackData();
		callback.m_Type = type;
		callback.m_UUID = uuid;
		callback.m_pResource = pResource;
		m_EnqueuedCallbacks.push_back(callback);
	}

	EditorAssetCallbacks::EditorAssetCallbacks() {}
	EditorAssetCallbacks::~EditorAssetCallbacks() {}
}
