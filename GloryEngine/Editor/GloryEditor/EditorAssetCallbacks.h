#pragma once
#include "UUID.h"
#include "ResourceMeta.h"
#include "Resource.h"
#include "ThreadedVar.h"
#include <functional>
#include <vector>
#include <GloryEditor.h>

#define ASSET_CALLBACK std::function<void(UUID, const ResourceMeta&, Resource*)>

namespace Glory:: Editor
{
	enum class AssetCallbackType : unsigned int
	{
		CT_AssetRegistered,
		CT_AssetDeleted,
		CT_AssetUpdated,
		CT_AssetDirty,

		CT_MAX,
	};

	class EditorAssetCallbacks
	{
	private:
		struct EnqueuedCallback
		{
			AssetCallbackType m_Type;
			UUID m_UUID;
			Resource* m_pResource;
		};

	public:
		static GLORY_EDITOR_API void RegisterCallback(const AssetCallbackType& type, ASSET_CALLBACK callback);
		static GLORY_EDITOR_API void EnqueueCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource);

	private:
		static void Initialize();
		static void Cleanup();
		static void RunCallbacks();

		static void TriggerCallback(const AssetCallbackType& type, UUID uuid, Resource* pResource);

	private:
		EditorAssetCallbacks();
		virtual ~EditorAssetCallbacks();

	private:
		friend class EditorAssetDatabase;
		static ThreadedVector<std::vector<ASSET_CALLBACK>> m_RegisteredCallbacks;
		static ThreadedVector<EnqueuedCallback> m_EnqueuedCallbacks;
	};
}
