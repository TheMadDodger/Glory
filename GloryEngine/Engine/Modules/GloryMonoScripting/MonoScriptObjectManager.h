#pragma once
#include <string>
#include <Glory.h>
#include "GloryMono.h"
#include "GloryMonoScipting.h"

namespace Glory
{
	struct ObjectInstanceData
	{
		MonoObject* m_pObject;
		uint32_t m_GCHandle;
		std::map<MonoClass*, MonoObject*> m_pObjects;
	};

	class MonoScriptObjectManager
	{
	public:
		static GLORY_API MonoObject* GetScriptDummyObject(MonoClass* pClass);
		static GLORY_API MonoObject* GetScriptObject(MonoClass* pClass, Object* pObject);
		static GLORY_API Object* GetScriptObject(MonoObject* pMonoObject);
		static GLORY_API void DestroyScriptObject(MonoClass* pClass, Object* pObject);

	private:
		static void Cleanup();
		static MonoObject* CreateScriptObject(MonoClass* pClass, Object* pObject);
		static MonoObject* CreateScriptDummyObject(MonoClass* pClass);

	private:
		friend class MonoManager;
		MonoScriptObjectManager();
		virtual ~MonoScriptObjectManager();
		static std::map<Object*, ObjectInstanceData> m_Objects;
		static std::map<MonoClass*, MonoObject*> m_pDummyScriptObjects;
		static std::map<MonoObject*, Object*> m_pMonoToObject;
	};
}
