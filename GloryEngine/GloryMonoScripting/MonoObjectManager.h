#pragma once
#include <string>
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
		static MonoObject* MonoObjectManager::GetObject(MonoClass* pClass, Object* pObject);
		static void DestroyObject(MonoClass* pClass, Object* pObject);

	private:
		static void Cleanup();
		static MonoObject* CreateObject(MonoClass* pClass, Object* pObject);

	private:
		MonoObjectManager();
		virtual ~MonoObjectManager();
		static std::map<Object*, ObjectInstanceData> m_Objects;
	};
}
