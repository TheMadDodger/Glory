#pragma once
#include <mono/jit/jit.h>
#include <UUID.h>
#include <Glory.h>

namespace Glory
{
	class MonoAssetManager
	{
	public:
		template<class AssetType>
		GLORY_API static MonoObject* MakeMonoAssetObject(UUID uuid)
		{
			std::string name = typeid(AssetType).name();
			name = name.erase(0, strlen("class Glory::"));
			size_t dataIndex = name.find("Data");
			name = name.erase(dataIndex, strlen("Data"));
			return MakeMonoAssetObject(uuid, "GloryEngine." + name);
		}

		GLORY_API static MonoObject* MakeMonoAssetObject(UUID uuid, const std::string& type);

	private:
		static void Initialize(MonoImage* pImage);

	private:
		MonoAssetManager();
		virtual ~MonoAssetManager();

	private:
		friend class CoreLibManager;
		static MonoClass* m_pAssetManagerClass;
		static MonoMethod* m_pGetResourceObjectMethod;
		static MonoMethod* m_pMakeResourceMethod;
	};
}
