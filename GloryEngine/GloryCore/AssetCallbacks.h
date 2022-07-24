#pragma once
#include "UUID.h"
#include "ResourceMeta.h"
#include "Resource.h"
#include "ThreadedVar.h"
#include <functional>
#include <vector>

#define ASSET_CALLBACK std::function<void(UUID, const ResourceMeta&, Resource*)>

namespace Glory
{
	enum class CallbackType : unsigned int
	{
		CT_AssetRegistered,
		CT_AssetLoaded,

		CT_AssetDeleted,

		CT_MAX,
	};

	class AssetCallbacks
	{
	private:
		struct EnqueuedCallback
		{
			CallbackType m_Type;
			UUID m_UUID;
			Resource* m_pResource;
		};

	public:
		static void RegisterCallback(const CallbackType& type, ASSET_CALLBACK callback);

	private:
		void Initialize();
		void Cleanup();
		void TriggerCallback(const CallbackType& type, UUID uuid, Resource* pResource);
		void RunCallbacks();
		void EnqueueCallback(const CallbackType& type, UUID uuid, Resource* pResource);

	private:
		AssetCallbacks();
		virtual ~AssetCallbacks();

	private:
		friend class AssetDatabase;
		friend class AssetManager;
		ThreadedVector<std::vector<ASSET_CALLBACK>> m_RegisteredCallbacks;
		ThreadedVector<EnqueuedCallback> m_EnqueuedCallbacks;
	};
}
