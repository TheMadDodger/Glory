#include "MonoAssetManager.h"
#include <AssetDatabase.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include "MonoManager.h"

namespace Glory
{
	MonoClass* MonoAssetManager::m_pAssetManagerClass = nullptr;
	MonoMethod* MonoAssetManager::m_pGetResourceObjectMethod = nullptr;
	MonoMethod* MonoAssetManager::m_pMakeResourceMethod = nullptr;

	MonoObject* MonoAssetManager::MakeMonoAssetObject(UUID uuid, const std::string& type)
	{
		if (uuid == 0) return nullptr;
		if (!AssetDatabase::AssetExists(uuid))
		{
			// Delete the mono version?
			return nullptr;
		}

		MonoString* pMonoString = mono_string_new(MonoManager::GetDomain(), type.c_str());
		void* args[2] = { &uuid, (void*)pMonoString };
		MonoObject* pExcept;
		return mono_runtime_invoke(m_pMakeResourceMethod, nullptr, args, &pExcept);
	}

	void MonoAssetManager::Initialize(MonoImage* pImage)
	{
		m_pAssetManagerClass = mono_class_from_name(pImage, "GloryEngine", "AssetManager");
		mono_class_init(m_pAssetManagerClass);

		MonoMethodDesc* pMethodDesc = mono_method_desc_new(".::GetResource(ulong)", false);
		m_pGetResourceObjectMethod = mono_method_desc_search_in_class(pMethodDesc, m_pAssetManagerClass);
		mono_method_desc_free(pMethodDesc);

		pMethodDesc = mono_method_desc_new(".::MakeResource(ulong,string)", false);
		m_pMakeResourceMethod = mono_method_desc_search_in_class(pMethodDesc, m_pAssetManagerClass);
		mono_method_desc_free(pMethodDesc);
	}

	MonoAssetManager::MonoAssetManager() {}
	MonoAssetManager::~MonoAssetManager() {}
}
