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
		GLORY_API MonoObject* GetScriptDummyObject(MonoClass* pClass);
		GLORY_API MonoObject* GetScriptObject(MonoClass* pClass, Object* pObject);
		GLORY_API Object* GetScriptObject(MonoObject* pMonoObject);
		GLORY_API void DestroyScriptObject(MonoClass* pClass, Object* pObject);

		GLORY_API void DestroyAllObjects();

	private:
		void Cleanup();
		MonoObject* CreateScriptObject(ObjectInstanceData& instanceData, MonoClass* pClass, Object* pObject);
		MonoObject* CreateScriptDummyObject(MonoClass* pClass);

	private:
		friend class AssemblyDomain;
		MonoScriptObjectManager();
		virtual ~MonoScriptObjectManager();
		std::map<Object*, ObjectInstanceData> m_Objects;
		std::map<MonoClass*, MonoObject*> m_pDummyScriptObjects;
		std::map<MonoObject*, Object*> m_pMonoToObject;
	};
}
