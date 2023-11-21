#pragma once
#include <string>
#include <Glory.h>
#include "GloryMono.h"
#include "GloryMonoScipting.h"

namespace Glory
{
	struct ObjectInstanceData
	{
		std::map<MonoClass*, std::pair<MonoObject*, uint32_t>> m_pObjects;
	};

	class MonoScriptObjectManager
	{
	public:
		GLORY_API MonoObject* GetMonoScriptDummyObject(MonoClass* pClass);
		GLORY_API MonoObject* GetMonoScriptObject(MonoClass* pClass, UUID uuid);
		GLORY_API UUID GetIDForMonoScriptObject(MonoObject* pMonoObject) const;
		GLORY_API void DestroyScriptObject(MonoClass* pClass, UUID uuid);

		GLORY_API void DestroyAllObjects();

	private:
		void Cleanup();
		MonoObject* CreateMonoScriptObject(ObjectInstanceData& instanceData, MonoClass* pClass, UUID uuid);
		MonoObject* CreateMonoScriptDummyObject(MonoClass* pClass);

	private:
		friend class AssemblyDomain;
		MonoScriptObjectManager();
		virtual ~MonoScriptObjectManager();
		std::map<UUID, ObjectInstanceData> m_Objects;
		std::map<MonoClass*, MonoObject*> m_pDummyScriptObjects;
		std::map<MonoObject*, UUID> m_pMonoToObject;
	};
}
