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

	class MonoObjectManager
	{
	public:
		static GLORY_API MonoObject* MonoObjectManager::GetDummyObject(MonoClass* pClass);
		static GLORY_API MonoObject* MonoObjectManager::GetObject(MonoClass* pClass, Object* pObject);
		static GLORY_API Object* MonoObjectManager::GetObject(MonoObject* pMonoObject);
		static GLORY_API void DestroyObject(MonoClass* pClass, Object* pObject);

	private:
		static void Cleanup();
		static MonoObject* CreateObject(MonoClass* pClass, Object* pObject);
		static MonoObject* CreateDummyObject(MonoClass* pClass);

	private:
		friend class MonoManager;
		MonoObjectManager();
		virtual ~MonoObjectManager();
		static std::map<Object*, ObjectInstanceData> m_Objects;
		static std::map<MonoClass*, MonoObject*> m_pDummyObjects;
		static std::map<MonoObject*, Object*> m_pMonoToObject;
	};
}
