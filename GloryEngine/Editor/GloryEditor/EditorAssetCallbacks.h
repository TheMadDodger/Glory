#pragma once
#include "UUID.h"
#include "ResourceMeta.h"
#include "Resource.h"

#include <ThreadedVar.h>
#include <functional>
#include <vector>
#include <GloryEditor.h>

namespace Glory:: Editor
{
	enum class AssetCallbackType : unsigned int
	{
		CT_AssetLoaded,
		CT_AssetReloaded,
		CT_AssetRegistered,
		CT_AssetDeleted,
		CT_AssetUpdated,
		CT_AssetDirty,

		CT_MAX,
	};

	struct AssetCallbackData
	{
		AssetCallbackType m_Type;
		UUID m_UUID;
		Resource* m_pResource;
	};

	class EditorAssetCallbacks
	{
	public:
		GLORY_EDITOR_API static UUID RegisterCallback(const AssetCallbackType& type, std::function<void(const AssetCallbackData&)> callback);
		GLORY_EDITOR_API static void RemoveCallback(const AssetCallbackType& type, UUID& id);
		GLORY_EDITOR_API static void EnqueueCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource);

	private:
		static void RunCallbacks();
		static void TriggerCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource);

	private:
		EditorAssetCallbacks();
		virtual ~EditorAssetCallbacks();

	private:
		friend class EditorAssetDatabase;
		static ThreadedVector<AssetCallbackData> m_EnqueuedCallbacks;
	};
}
