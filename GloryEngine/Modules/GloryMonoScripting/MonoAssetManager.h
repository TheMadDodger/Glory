#pragma once
#include <mono/jit/jit.h>
#include <UUID.h>
#include <Glory.h>
#include <string>
#include <map>

namespace Glory
{
	class Engine;

	class MonoAssetManager
	{
	public:
		template<class AssetType>
		GLORY_API static MonoObject* MakeMonoAssetObject(Engine* pEngine, UUID uuid)
		{
			std::string name = typeid(AssetType).name();
			name = name.erase(0, strlen("class Glory::"));
			size_t dataIndex = name.find("Data");
			name = name.erase(dataIndex, strlen("Data"));
			return MakeMonoAssetObject(pEngine, uuid, "GloryEngine." + name);
		}

		GLORY_API static MonoObject* MakeMonoAssetObject(Engine* pEngine, UUID uuid, const std::string& type);

	private:
		static void Initialize(MonoImage* pImage);
		static void Cleanup();

		/* TODO: Cleanup */

	private:
		MonoAssetManager();
		virtual ~MonoAssetManager();

	private:
		friend class CoreLibManager;
		friend class MonoManager;
		static MonoClass* m_pAssetManagerClass;
		static MonoMethod* m_pGetResourceObjectMethod;
		static MonoMethod* m_pMakeResourceMethod;
		static MonoMethod* m_pClearCacheMethod;
		static std::map<UUID, MonoObject*> m_AssetCache;
	};
}
